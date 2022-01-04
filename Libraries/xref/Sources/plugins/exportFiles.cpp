///*
// * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
// *
// * @APPLE_BSD_LICENSE_HEADER_START@
// *
// * Redistribution and use in source and binary forms, with or without
// * modification, are permitted provided that the following conditions
// * are met:
// *
// * 1.  Redistributions of source code must retain the above copyright
// *     notice, this list of conditions and the following disclaimer.
// * 2.  Redistributions in binary form must reproduce the above copyright
// *     notice, this list of conditions and the following disclaimer in the
// *     documentation and/or other materials provided with the distribution.
// * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
// *     its contributors may be used to endorse or promote products derived
// *     from this software without specific prior written permission.
// *
// * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
// * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
// * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// *
// * @APPLE_BSD_LICENSE_HEADER_END@
// */
//
//#include "DBDataStore.h"
//#include <PluginManager.h>
//#include <sys/stat.h>
//#include <stdio.h>
//#include <vector>
//#include <regex.h>
//
//static int exportFiles(SqlDatabase dbm, std::string project);
//
//static int run(SqlDatabase dbm, std::vector<std::string> argv) {
//	int res = 0;
//    size_t count = argv.size();
//	if (count > 1)  return -1;
//
//    std::string project = "";
//	if (count == 1) {
//        project = argv.at(0);
//	}
//	exportFiles(dbm, project);
//	return res;
//}
//
//static std::string usage() {
//	return "[<project>]";
//}
//
//int initialize(int version) {
//	//if ( version < kDBPluginCurrentVersion ) return -1;
//    Plugin plugin;
//    plugin.setType(PluginType::BasicType);
//    plugin.setName("exportFiles");
//    plugin.setRunFunc(&run);
//    plugin.setUsageFunc(&usage);
//	return 0;
//}
//
//int printFiles(void* pArg, int argc, char **argv, char** columnNames) {
//	fprintf(stdout, "\t%s\n", argv[0]);
//	return 0;
//}
//
//static int exportFiles(SqlDatabase dbm, std::string project) {
//	int res;
//
//	char* table = "CREATE TABLE files (build text, project text, path text)";
//	char* index = "CREATE INDEX files_index ON files (build, project, path)";
//    dbm.query_noerr(table);
//	dbm.query_noerr(index);
//
//    std::cout << "# BUILD " << dbm.m_build << std::endl;
//
//    if (!project.empty()) {
//        std::cout << project << ":"<< std::endl;
//        res = dbm.query(printFiles, NULL,
//                        "SELECT path FROM files WHERE build=%Q AND project=%Q", dbm.m_build.c_str(), project.c_str());
//	} else {
//		CFArrayRef projects = DBCopyProjectNames(DBGetCurrentBuild());
//		if (projects) {
//			CFIndex i, count = CFArrayGetCount(projects);
//			for (i = 0; i < count; ++i) {
//				CFStringRef name = CFArrayGetValueAtIndex(projects, i);
//				char* project = strdup_cfstr(name);
//				fprintf(stdout, "%s:\n", project);
//				res = SQL_CALLBACK(&printFiles, NULL,
//					"SELECT path FROM files WHERE build=%Q AND project=%Q",
//					build, project);
//				free(project);
//			}
//			CFRelease(projects);
//		}
//	}
//
//	return 0;
//}
