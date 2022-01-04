/*-
 * SPDX-License-Identifier: BSD-3-Clause
 **
 * This code is derived from software contributed to Pure Darwin
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Plugin.h"
#include <iostream>

namespace plugin {

class Version : PropertyPlugin<DBArray<DBString>> {
public:
     Version(): PropertyPlugin("version")  {}
    ~Version()                             = default;

    int run(DBDatabase &db, const std::string& build, const std::vector<std::string>& argv) override {
        std::vector<std::string> data;
        std::string project;
        std::string version;
        int ret = 0;

        if (argv.size() != 1)
            return -1;

        project = argv.at(0);
        if (project == "*") {
            std::vector<std::string> projects = db.getProjectNames(build, false);
            for (auto &iter : projects) {
                version = db.getString(build, iter, "version");
                if (!version.empty()) {
                  std::cout << iter << "-" << version << std::endl;
                }
            }
        } else {
          version = db.getString(build, project, "version");
          if (!version.empty()) {
            std::cout << version << std::endl;
          } else {
            std::cout << "The project \""<< project <<"\" doesn't exist" << std::endl;
          }
        }

        return ret;
    }

    std::string usage() override {
        return "<project>\n"
               "*: all projects for current build\n";
        // maybe create an option like "darwinwref version * xnu"
        // to list all xnu versions for every available PureDarwnin build
    }
};

DECLARE_PLUGIN(Version, version);

}
