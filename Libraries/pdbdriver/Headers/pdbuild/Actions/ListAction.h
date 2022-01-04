/**
Copyright (c) 2015-present, Facebook, Inc.
All rights reserved.

This source code is licensed under the BSD-style license found in the
                                                    LICENSE file in the root directory of this source tree.
                                                        */

#ifndef pdbuild_ListAction_h
#define pdbuild_ListAction_h

namespace libutil { class Filesystem; }
namespace process { class PDBContext; }
namespace process { class Launcher; }
namespace process { class User; }

namespace pdbuild {

class Options;

class ListAction {
private:
  ListAction();
 ~ListAction();

public:
 static int
  Run(process::User const *user, process::PDBContext const *processContext, process::Launcher *processLauncher, libutil::Filesystem *filesystem, Options const &options);
};

}

#endif // !pdbuild_ListAction_h
