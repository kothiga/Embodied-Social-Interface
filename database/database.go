/* ================================================================================
 * Copyright: (C) 2022, SIRRL Social and Intelligent Robotics Research Laboratory,
 *     University of Waterloo, All rights reserved.
 *
 * Authors:
 *     Austin Kothig <austin.kothig@uwaterloo.ca>
 *
 * CopyPolicy: Released under the terms of the MIT License.
 *     See the accompanying LICENSE file for details.
 * ================================================================================
 */

package database

import (
	"database/sql"
	"fmt"
	"strconv"
	"strings"
	"sync"

	_ "github.com/go-sql-driver/mysql"
	"github.com/google/uuid"
)

var mu = new(sync.Mutex)
var db *sql.DB

func InitDataBase() {
	fmt.Printf("Opening Database\n")

	var err error
	db, err = sql.Open("mysql", "austin:T2jkv*H2@/embodTest")
	db.SetMaxOpenConns(200)
	db.SetMaxIdleConns(200)
	panicError(err)
	createDataBase()
}

func createDataBase() {
	mu.Lock()
	defer mu.Unlock()

	var err error
	_, err = db.Exec(createUserStmt)
	panicError(err)
	//_, err = db.Exec(createEmotionStmt)
	//panicError(err)
	//_, err = db.Exec(createDemographicStmt)
	//panicError(err)
	//_, err = db.Exec(createUnderstandingStmt)
	//panicError(err)
	//_, err = db.Exec(createIDAQStmt)
	//panicError(err)
	//_, err = db.Exec(createPre1Stmt)
	//panicError(err)
	//_, err = db.Exec(createPre2Stmt)
	//panicError(err)
	//_, err = db.Exec(createPostStmt)
	//panicError(err)
	//_, err = db.Exec(createFinalStmt)
	//panicError(err)
	//_, err = db.Exec(createCheckStmt)
	//panicError(err)
}

var createUserStmt = `CREATE TABLE IF NOT EXISTS user (
	session_id VARCHAR(32) NOT NULL,
	mturk_id VARCHAR(256) NOT NULL,
	hit_id VARCHAR(256) NOT NULL,
	current_emotion VARCHAR(256) NOT NULL,
	order_num VARCHAR(256) NOT NULL,
	group_num VARCHAR(256) NOT NULL,
	cannot_load INT NOT NULL DEFAULT 0,
	replay INT NOT NULL DEFAULT 0,
	play_comment INT NOT NULL DEFAULT 0,
	created TIMESTAMP NOT NULL,
	updated TIMESTAMP NOT NULL,
	UNIQUE KEY(session_id),
	PRIMARY KEY(mturk_id)
);`

// TODO: replace current_emotion for current_channel?

//
// Some helper functions.
//
func closeRows(result *sql.Rows) {
	err := result.Close()
	reportError(err)
}

func reportError(err error) {
	if err != nil {
		fmt.Println(err)
	}
}

func panicError(err error) {
	if err != nil {
		panic(err)
	}
}

func newUUID() string {
	return strings.Replace(uuid.New().String(), "-", "", -1)
}

//
// Create a user in the table.
//

// Queries
var getUserStmt = `SELECT session_id FROM user WHERE mturk_id = ?;`
var addUserStmt = `INSERT INTO user (
	session_id, mturk_id, hit_id, current_emotion, order_num, group_num, created, updated
	) VALUES (?, ?, ?, '', ?, ?, NOW(), NOW());`

// Global var
var ord_count = 0

func CreateUser(mturkId, hitId, group string) string {
	mu.Lock()
	defer mu.Unlock()

	getUserResult, err := db.Query(getUserStmt, mturkId)
	reportError(err)
	defer closeRows(getUserResult)

	oldSessionId := "error"

	ord_count = ord_count + 1

	if getUserResult.Next() {
		err = getUserResult.Scan(&oldSessionId)
		reportError(err)
		//return oldSessionId
		return "error" // reject re-entries
	}

	sessionId := newUUID()
	_, err = db.Exec(addUserStmt, sessionId, mturkId, hitId, ord_count, group)
	reportError(err)

	return sessionId + "&n=" + strconv.Itoa(ord_count)
}

//
// Get a subject from their session id.
//

// Query
var getSubjectStmt = `SELECT current_emotion FROM user WHERE session_id = ?;`

func GetSubject(sessionId string) string {
	fmt.Println("GetSubject", sessionId)
	mu.Lock()
	defer mu.Unlock()

	getSubjectResult, err := db.Query(getSubjectStmt, sessionId)
	reportError(err)
	defer closeRows(getSubjectResult)

	if getSubjectResult.Next() {
		subject := ""
		err = getSubjectResult.Scan(&subject)
		reportError(err)
		return subject
	}

	return ""
}

//
// Get completed subjects.
//

// Query
var getCompletedSubjectsStmt = `SELECT emotion FROM emotion WHERE session_id = ?;`

func GetCompletedSubjects(sessionId string) map[string]bool {
	fmt.Println("GetCompletedSubjects", sessionId)
	mu.Lock()
	defer mu.Unlock()

	getCompletedSubjectsResult, err := db.Query(getCompletedSubjectsStmt, sessionId)
	reportError(err)
	defer closeRows(getCompletedSubjectsResult)

	result := make(map[string]bool)

	for getCompletedSubjectsResult.Next() {
		subject := ""
		err = getCompletedSubjectsResult.Scan(&subject)
		reportError(err)
		result[subject] = true
	}

	return result
}

//
// Update a subject.
//

// Query
var updateSubjectStmt = `UPDATE user SET current_emotion = ? WHERE session_id = ?;`

func UpdateSubject(sessionId, subject string) {
	fmt.Println("UpdateSubject", sessionId, subject)
	mu.Lock()
	defer mu.Unlock()

	_, err := db.Exec(updateSubjectStmt, subject, sessionId)
	reportError(err)
}

//
//
//

// Query
var getEntryStmt = `SELECT * FROM emotion WHERE session_id = ? AND emotion = ?;`

func EntryExists(sessionId, subject string) bool {
	fmt.Println("EntryExists", sessionId, subject)
	mu.Lock()
	defer mu.Unlock()

	fmt.Println("exists", sessionId, subject)

	getEntryResult, err := db.Query(getEntryStmt, sessionId, subject)
	defer closeRows(getEntryResult)

	reportError(err)
	return getEntryResult.Next()
}

func CreateEntry(sessionId, subject string) {
	fmt.Println("CreateEntry", sessionId, subject)
	mu.Lock()
	defer mu.Unlock()

	countEntryResult, err := db.Query(countEntryStmt, sessionId)
	defer closeRows(countEntryResult)
	entryCount := 0
	reportError(err)
	if countEntryResult.Next() {
		err = countEntryResult.Scan(&entryCount)
	}
	fmt.Println("create", sessionId, subject, entryCount+1)

	_, err = db.Exec(createEntryStmt, sessionId, subject, entryCount+1)
	reportError(err)
}

var countEntryStmt = `SELECT COUNT(*) FROM emotion WHERE session_id = ?;`

var createEntryStmt = `INSERT INTO emotion (session_id, emotion, order_num, created, updated) VALUES
(?, ?, ?, NOW(), NOW());`
