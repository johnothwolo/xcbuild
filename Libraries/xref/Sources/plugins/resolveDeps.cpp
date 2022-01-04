/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_BSD_LICENSE_HEADER_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * @APPLE_BSD_LICENSE_HEADER_END@
 */

#include <sys/stat.h>
#include <sys/types.h>

#include "Plugin.h"

namespace plugin {

class ResolveDeps : BasicPlugin<DBString> {
public:
    ResolveDeps() : BasicPlugin("resolveDeps") {}
   ~ResolveDeps()                              {}
    
    int run(DBDatabase &db, const std::string& build, const std::vector<std::string>& argv) override {
        std::vector<std::string> data;
        size_t count = argv.size();
        std::string project;
        std::string version;
        int commit = 0;

        if (count > 0) {
            if (argv[0] == std::string("-commit")) {
            commit = 1;
          } else {
            project = argv.at(0);
          }
        }

        if (count > 1) {
            project = argv[1];
        }

        resolve_dependencies(db, build, project, commit);
        return 0;
    };
    
    std::string usage() override {
        return "[-commit] [<project>]";
    };
    
    int resolve_dependencies(DBDatabase &db, const std::string& build, const std::string& project, int commit){
        std::vector<std::string> builds;
        std::vector<std::string> projects;
        int resolvedCount = 0, unresolvedCount = 0;
        // if committing, use all projects, otherwise use unresolved projects
        std::string table = (char*)( commit ? "properties" : "unresolved_dependencies" );

        //
        // If no project, version specified, resolve everything.
        // Otherwise, resolve only that project or version.
        //
        if (project.empty()) {
            builds = db.queryArray<std::string>("SELECT DISTINCT build FROM "+table+" WHERE project IS NOT NULL");
            projects = db.queryArray<std::string>("SELECT DISTINCT project FROM "+table+" WHERE project IS NOT NULL");
        } else {
            builds = db.queryArray<std::string>("SELECT DISTINCT build FROM "+table+" WHERE project="+project);
            projects = db.queryArray<std::string>("SELECT DISTINCT project FROM "+table+" WHERE project="+project);
        }
        
        unsigned long count = projects.size();
        for (int i = 0; i < count; ++i) {
            auto build = builds[i];
            auto project = projects[i];
            std::cerr << project <<" (" << build << ")\n";
            resolve_project_dependencies(db, build, project, resolvedCount, unresolvedCount, commit);
        }

        std::cout << resolvedCount <<" dependencies resolved, "<< unresolvedCount <<" remaining." << std::endl;
        
        return 0;
    };

    int resolve_project_dependencies(DBDatabase &db, std::string build, std::string project, int& resolvedCount, int& unresolvedCount, int commit) {
        std::vector<std::string> files;
        std::vector<std::string> types;

        db.query("CREATE TABLE dependencies (build TEXT, project TEXT, type TEXT, dependency TEXT)");
        db.query("CREATE INDEX dependencies_index ON unresolved_dependencies (build, project, type, dependency)");
        
        db.beginTransaction();

        // Convert from unresolved_dependencies (i.e. path names) to resolved dependencies (i.e. project names)
        // Deletes unresolved_dependencies after they are processed.
        files = db.queryArray<std::string>("SELECT DISTINCT dependency FROM unresolved_dependencies WHERE build="
                                          +build+" AND project="+project);
        types = db.queryArray<std::string>("SELECT DISTINCT type FROM unresolved_dependencies WHERE build="
                                          +build+" AND project="+project);

        for (int i = 0; i < files.size(); ++i) {
            std::string file = files.at(i);
            std::string type = types.at(i);
            // XXX
            // This assumes a 1-to-1 mapping between files and projects.
            std::string dep = db.query<std::string>("SELECT project FROM files WHERE path="+file);
            if (!dep.empty()) {
                // don't add duplicates
                bool exists = db.query<bool>("SELECT 1 FROM dependencies WHERE build="+build+" AND project="
                                             +project+" AND type="+type+" AND dependency="+dep);
                if (!exists) {
                    db.query("INSERT INTO dependencies (build,project,type,dependency) VALUES"
                             "("+build+","+project+","+type+","+dep+")");
                    resolvedCount += 1;
                    std::cerr << "\t" << dep << "(" << type << ")\n";
                }
                db.query("DELETE FROM unresolved_dependencies WHERE"
                         "build="+build+" AND project="+project+" AND type="+type+" AND dependency="+file);
            } else {
                unresolvedCount += 1;
            }
        }

        // If committing, merge resolved dependencies to the dependencies property dictionary.
        // Deletes resolved dependencies after they are processed.
        if (commit) {
            std::vector<std::string> types;
            std::vector<std::string> projs;
            std::vector<std::vector<std::string>> params = { projs, types };
            auto dependencies = db.getDictionaryProp<std::string,std::vector<std::string>>(build, project, "dependencies");
            
            db.queryArray<std::string>("SELECT DISTINCT dependency,type FROM dependencies WHERE"
                                        "build="+build+" AND project="+project);
            
            for (int i = 0; i < projs.size(); ++i) {
                std::string proj = projs.at(i);
                std::string type = types.at(i);
                
                auto deparray = dependencies[type];
                if (deparray.empty()) {
                    dependencies[type] = deparray;
                }
                
                if (std::find(deparray.begin(), deparray.end(), proj) != deparray.end()) {
                    deparray.push_back(proj);
                }
            }
            
//            DBSetProp(cfstr(build), cfstr(project), CFSTR("dependencies"), dependencies);

            db.query("DELETE FROM dependencies WHERE build="+build+
                     " AND project="+project);
        }

        db.commitTransaction();
        return 0;
    }
    
};

DECLARE_PLUGIN(ResolveDeps, resolveDeps);

} // namespace plugin

#if 0


int resolve_project_dependencies( const char* build, const char* project, int* resolvedCount, int* unresolvedCount, int commit) {
	CFMutableArrayRef files = CFArrayCreateMutable(NULL, 0, &cfArrayCStringCallBacks);
	CFMutableArrayRef types = CFArrayCreateMutable(NULL, 0, &cfArrayCStringCallBacks);
	CFMutableArrayRef params[2] = { files, types };

        char* table = "CREATE TABLE dependencies (build TEXT, project TEXT, type TEXT, dependency TEXT)";
        char* index = "CREATE INDEX dependencies_index ON unresolved_dependencies (build, project, type, dependency)";

        SQL_NOERR(table);
        SQL_NOERR(index);

	if (SQL("BEGIN")) { return -1; }

	// Convert from unresolved_dependencies (i.e. path names) to resolved dependencies (i.e. project names)
	// Deletes unresolved_dependencies after they are processed.
	SQL_CALLBACK(&addToCStrArrays, params,
		"SELECT DISTINCT dependency,type FROM unresolved_dependencies WHERE build=%Q AND project=%Q",
		build, project);

	CFIndex i, count = CFArrayGetCount(files);
	for (i = 0; i < count; ++i) {
		const char* file = CFArrayGetValueAtIndex(files, i);
		const char* type = CFArrayGetValueAtIndex(types, i);
		// XXX
		// This assumes a 1-to-1 mapping between files and projects.
		char* dep = (char*)SQL_STRING("SELECT project FROM files WHERE path=%Q", file);
		if (dep) {
			// don't add duplicates
			int exists = SQL_BOOLEAN("SELECT 1 FROM dependencies WHERE build=%Q AND project=%Q AND type=%Q AND dependency=%Q",
				build, project, type, dep);
			if (!exists) {
				SQL("INSERT INTO dependencies (build,project,type,dependency) VALUES (%Q,%Q,%Q,%Q)",
					build, project, type, dep);
				*resolvedCount += 1;
				fprintf(stderr, "\t%s (%s)\n", dep, type);
			}
			SQL("DELETE FROM unresolved_dependencies WHERE build=%Q AND project=%Q AND type=%Q AND dependency=%Q",
				build, project, type, file);
		} else {
			*unresolvedCount += 1;
		}
	}

	CFRelease(files);
	CFRelease(types);

	// If committing, merge resolved dependencies to the dependencies property dictionary.
	// Deletes resolved dependencies after they are processed.
	if (commit) {
		CFMutableArrayRef types = CFArrayCreateMutable(NULL, 0, &cfArrayCStringCallBacks);
		CFMutableArrayRef projs = CFArrayCreateMutable(NULL, 0, &cfArrayCStringCallBacks);
		CFMutableArrayRef params[2] = { projs, types };
		CFMutableDictionaryRef dependencies = (CFMutableDictionaryRef)DBCopyPropDictionary(cfstr(build), cfstr(project), CFSTR("dependencies"));
		if (dependencies == NULL) {
			dependencies = CFDictionaryCreateMutable(NULL, 0,
								     &kCFCopyStringDictionaryKeyCallBacks,
								     &kCFTypeDictionaryValueCallBacks);
		}
		
		SQL_CALLBACK(&addToCStrArrays, params,
			"SELECT DISTINCT dependency,type FROM dependencies WHERE build=%Q AND project=%Q",
			build, project);
		
		CFIndex i, count = CFArrayGetCount(projs);
		for (i = 0; i < count; ++i) {
			CFStringRef proj = cfstr(CFArrayGetValueAtIndex(projs, i));
			CFStringRef type = cfstr(CFArrayGetValueAtIndex(types, i));
			
			CFMutableArrayRef deparray = (CFMutableArrayRef)CFDictionaryGetValue(dependencies, type);
			if (deparray == NULL) {
				deparray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
				CFDictionarySetValue(dependencies, type, deparray);
				CFRelease(deparray); // still retained by dict
			}
			if (!CFArrayContainsValue(deparray,
							CFRangeMake(0, CFArrayGetCount(deparray)),
							proj)) {
				CFArrayAppendValue(deparray, proj);
			}
			CFRelease(proj);
			CFRelease(type);
		}
		
		DBSetProp(cfstr(build), cfstr(project), CFSTR("dependencies"), dependencies);
		CFRelease(dependencies);
		CFRelease(types);
		CFRelease(projs);

		SQL("DELETE FROM dependencies WHERE build=%Q AND project=%Q", build, project);
	}

	if (SQL("COMMIT")) { return -1; }

	return 0;
}


#endif
