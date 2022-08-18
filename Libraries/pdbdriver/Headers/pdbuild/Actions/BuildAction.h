/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#ifndef pdbuild_BuildAction_h
#define pdbuild_BuildAction_h

#include <string>

namespace libutil { class Filesystem; }
namespace process { class PDBContext; }
namespace process { class Launcher; }
namespace process { class User; }

namespace pdbuild {

class Options;

class BuildAction {
public:
  class BuildTool {
  public:
    static const std::string Invalid;
    static const std::string Xcode;
    static const std::string Makefile;
    static const std::string CMake;
    static const std::string Ninja;
  };

private:
    BuildAction();
    ~BuildAction();

public:
    static int
    Run(process::User const *user, process::PDBContext const *processContext, process::Launcher *processLauncher, libutil::Filesystem *filesystem, Options const &options);
};

}

#endif // !pdbuild_BuildAction_h
