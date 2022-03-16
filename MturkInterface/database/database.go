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
	_, err = db.Exec(createMovesStmt)
	panicError(err)
}

var createUserStmt = `CREATE TABLE IF NOT EXISTS user (
	session_id      VARCHAR(32)  NOT NULL,
	user_id         VARCHAR(256) NOT NULL,
	current_hash    VARCHAR(256) NOT NULL,
	current_channel VARCHAR(256) NOT NULL,
	order_num       VARCHAR(256) NOT NULL,
	cannot_load     INT          NOT NULL DEFAULT 0,
	replay          INT          NOT NULL DEFAULT 0,
	play_comment    INT          NOT NULL DEFAULT 0,
	created         TIMESTAMP    NOT NULL,
	updated         TIMESTAMP    NOT NULL,
	UNIQUE   KEY(session_id),
	PRIMARY  KEY(user_id)
);`

var createMovesStmt = `CREATE TABLE IF NOT EXISTS user (
	session_id      VARCHAR(32)  NOT NULL,
	current_channel VARCHAR(256) NOT NULL,
	hint_id         VARCHAR(256) NOT NULL,
	current_hash    VARCHAR(256) NOT NULL,
	goal_distance   INT          NOT NULL DEFAULT -1,
	move_number     INT          NOT NULL DEFAULT -1,
	u               INT          NOT NULL DEFAULT -1,
	v               INT          NOT NULL DEFAULT -1,
	cannot_load     INT          NOT NULL DEFAULT 0,
	replay          INT          NOT NULL DEFAULT 0,
	play_comment    INT          NOT NULL DEFAULT 0,
	created         TIMESTAMP    NOT NULL,
	updated         TIMESTAMP    NOT NULL,
	UNIQUE   KEY(session_id),
	PRIMARY  KEY(user_id)
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
var getUserStmt = `SELECT session_id FROM user WHERE user_id = ?;`
var addUserStmt = `INSERT INTO user (
		session_id, user_id, current_hash, current_channel, order_num, created, updated
	) VALUES (
		?, ?, '', '', ?, NOW(), NOW()
);`

// Global var
var order_count = 0

func CreateUser(userId, currentHash string) string {
	mu.Lock()
	defer mu.Unlock()

	getUserResult, err := db.Query(getUserStmt, userId)
	reportError(err)
	defer closeRows(getUserResult)

	oldSessionId := "error"

	order_count = order_count + 1

	if getUserResult.Next() {
		err = getUserResult.Scan(&oldSessionId)
		reportError(err)
		//return oldSessionId // allow re-entries?
		return "error" // reject re-entries
	}

	sessionId := newUUID()
	_, err = db.Exec(addUserStmt, sessionId, userId, order_count)
	reportError(err)

	return sessionId + "&n=" + strconv.Itoa(order_count)
}

// //
// // Get a subjects current hash from their session id.
// //
//
// var getSubjectStmt = `SELECT current_hash FROM user WHERE session_id = ?;`
//
// func GetSubject(sessionId string) string {
// 	fmt.Println("GetSubject", sessionId)
// 	mu.Lock()
// 	defer mu.Unlock()
//
// 	getSubjectResult, err := db.Query(getSubjectStmt, sessionId)
// 	reportError(err)
// 	defer closeRows(getSubjectResult)
//
// 	if getSubjectResult.Next() {
// 		subject := ""
// 		err = getSubjectResult.Scan(&subject)
// 		reportError(err)
// 		return subject
// 	}
//
// 	return ""
// }
//
//
// //
// // Update a subject.
// //
//
// // Query
// var updateSubjectStmt = `UPDATE user SET current_emotion = ? WHERE session_id = ?;`
//
// func UpdateSubject(sessionId, subject string) {
// 	fmt.Println("UpdateSubject", sessionId, subject)
// 	mu.Lock()
// 	defer mu.Unlock()
//
// 	_, err := db.Exec(updateSubjectStmt, subject, sessionId)
// 	reportError(err)
// }
//
// //
// //
// //
//
// // Query
// var getEntryStmt = `SELECT * FROM emotion WHERE session_id = ? AND emotion = ?;`
//
// func EntryExists(sessionId, subject string) bool {
// 	fmt.Println("EntryExists", sessionId, subject)
// 	mu.Lock()
// 	defer mu.Unlock()
//
// 	fmt.Println("exists", sessionId, subject)
//
// 	getEntryResult, err := db.Query(getEntryStmt, sessionId, subject)
// 	defer closeRows(getEntryResult)
//
// 	reportError(err)
// 	return getEntryResult.Next()
// }
//
// func CreateEntry(sessionId, subject string) {
// 	fmt.Println("CreateEntry", sessionId, subject)
// 	mu.Lock()
// 	defer mu.Unlock()
//
// 	countEntryResult, err := db.Query(countEntryStmt, sessionId)
// 	defer closeRows(countEntryResult)
// 	entryCount := 0
// 	reportError(err)
// 	if countEntryResult.Next() {
// 		err = countEntryResult.Scan(&entryCount)
// 	}
// 	fmt.Println("create", sessionId, subject, entryCount+1)
//
// 	_, err = db.Exec(createEntryStmt, sessionId, subject, entryCount+1)
// 	reportError(err)
// }
//
// var countEntryStmt = `SELECT COUNT(*) FROM emotion WHERE session_id = ?;`
//
// var createEntryStmt = `INSERT INTO emotion (session_id, emotion, order_num, created, updated) VALUES
// (?, ?, ?, NOW(), NOW());`
//
