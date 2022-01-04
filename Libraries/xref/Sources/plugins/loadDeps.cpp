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
#include <sys/param.h>

#include "Plugin.h"

static int has_suffix(const char* big, const char* little) {
    char* found = NULL;
    while (1) {
        char* next = strcasestr(found ? found+1 : big, little);
        if (next) { found = next; } else { break; }
    }
    return found ? (strcmp(found, little) == 0) : 0;
}

namespace plugin {

class LoadDeps : BasicPlugin<DBString> {
public:
    LoadDeps() : BasicPlugin("loadDeps") {}
   
    std::string usage() override {
        return "<project> <buildroot>";
    }
    
    int run(DBDatabase &db, const std::string& build, const std::vector<std::string>& argv) override {
        if (argv.size() != 2)  return -1;
        
        std::string project = argv[0];
        std::string root = argv[1];
                
        return loadDeps(db, build, project, root);
    }
private:
    int loadDeps(DBDatabase &db, std::string build, std::string project, std::string root){
        size_t size;
        char* line;
        int count = 0;

        db.query("CREATE TABLE unresolved_dependencies (build TEXT, project TEXT, type TEXT, dependency TEXT)");
        db.query("CREATE INDEX unresolved_dependencies_index"
                 "ON unresolved_dependencies (build, project, type, dependency)");
        
        db.beginTransaction();
        
        while ((line = fgetln(stdin, &size)) != NULL) {
            if (line[size-1] == '\n') line[size-1] = 0; // chomp newline
            char* tab = (char*)memchr(line, '\t', size);
            if (tab) {
                std::string fullpath;
                std::string type;
                std::string file;
                struct stat sb;
                int typesize = (int)((intptr_t)tab - (intptr_t)line);
                
                fullpath.reserve(MAXPATHLEN);
                
                type +=  typesize + line;
                file += ((int)size - typesize - 1) + tab+1;
                if (type == "open") {
                    if (has_suffix(file.c_str(), ".h")) {
                        type = "header";
                    } else if (has_suffix(file.c_str(), ".a") || has_suffix(file.c_str(), ".o")) {
                        type = "staticlib";
                    } else {
                        type = "build";
                    }
                } else if (type == "execve" || type == "readlink") {
                    type = "build";
                }
                fullpath += root + "/" + file;
                int res = lstat(fullpath.c_str(), &sb);
                // for now, skip if the path points to a directory
                if (res == 0 && !S_ISDIR(sb.st_mode)) {
                    db.query("INSERT INTO unresolved_dependencies (build,project,type,dependency) "
                             "VALUES ("+build+","+project+","+type+","+file+")");
                }
            } else {
                std::cerr << "Error: syntax error in input.  no tab delimiter found." << std::endl;
            }
            ++count;
        }

        db.commitTransaction();

        std::cerr << "loaded " << count << " unresolved dependencies." << std::endl;
        return 0;
    }
};

DECLARE_PLUGIN(LoadDeps, loadDeps);

} // namespace plugin
