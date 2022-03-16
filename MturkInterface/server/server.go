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

package server

import (
	"EmbodInterface/database"
	"fmt"
	"log"
	"net/http"
	"strconv"
)

//
// Start the server.
//
func Start() {

	// Init the file server to fetch static files from.
	fileServer := noDirListing(http.FileServer(http.Dir("server/static")))
	http.Handle("/", fileServer)

	http.HandleFunc("/create", createHandler)

	//http.HandleFunc("/check", checkHandler)
	//http.HandleFunc("/comment", commentHandler)
	//http.HandleFunc("/complete", completeHandler)

	//http.HandleFunc("/demographics", demographicsHandler)
	//http.HandleFunc("/final", finalHandler)
	//http.HandleFunc("/picture", pictureHandler)
	//http.HandleFunc("/post", postHandler)
	//http.HandleFunc("/replay", replayHandler)
	//http.HandleFunc("/select", selectHandler)
	//http.HandleFunc("/subject", subjectHandler)
	//http.HandleFunc("/understanding", understandingHandler)
	//http.HandleFunc("/watch_again", watchAgainHandler)

	//http.HandleFunc("/idaq", idaqHandler)
	//http.HandleFunc("/tipi", tipiHandler)
	//http.HandleFunc("/dtop", dtopHandler)

	fmt.Printf("Starting server at port 8080...\n")
	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatal(err)
	}
}

//
// Some helper functions.
//
func reportError(err error) {
	if err != nil {
		fmt.Println(err)
	}
}

func stringInSlice(a string, list []string) bool {
	for _, b := range list {
		if b == a {
			return true
		}
	}
	return false
}

func noDirListing(h http.Handler) http.HandlerFunc {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		h.ServeHTTP(w, r)
	})
}

func getUrlParam(r *http.Request, name string) string {
	if name == "" {
		return ""
	}
	query := r.URL.Query()
	params := query[name]
	if len(params) == 0 || params[0] == "" {
		return ""
	}
	return params[0]
}

func getUrlBoolParam(r *http.Request, name string) bool {
	return getUrlParam(r, name) == "1"
}

func getUrlIntParam(r *http.Request, name string) int {
	str := getUrlParam(r, name)
	val, err := strconv.Atoi(str)
	reportError(err)
	return val
}

//
// Handler for /create
//
func createHandler(w http.ResponseWriter, r *http.Request) {
	userId := getUrlParam(r, "user_id")
	currentHash := getUrlParam(r, "current_hash")
	//group := getUrlParam(r, "group")
	sessionId := database.CreateUser(userId, currentHash)
	_, err := w.Write([]byte(sessionId))
	reportError(err)
}
