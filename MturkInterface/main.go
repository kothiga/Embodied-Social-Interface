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

package main

import (
	"EmbodInterface/database"
	"EmbodInterface/server"
)

func main() {
	database.InitDataBase()
	server.Start()
}
