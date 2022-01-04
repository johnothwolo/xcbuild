/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#ifndef pdbuild_VersionAction_h
#define pdbuild_VersionAction_h

namespace libutil { class Filesystem; }
namespace process { class PDBContext; }
namespace process { class Launcher; }
namespace process { class User; }

namespace pdbuild {

class Options;

class VersionAction {
private:
    VersionAction();
   ~VersionAction();

public:
    static int
    Run(process::User const *user, process::PDBContext const *processContext, libutil::Filesystem const *filesystem, Options const &options);
};

}

#endif // !pdbuild_VersionAction_h
