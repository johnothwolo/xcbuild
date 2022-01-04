/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#ifndef pdbuild_LicenseAction_h
#define pdbuild_LicenseAction_h

namespace libutil { class Filesystem; }
namespace process { class PDBContext; }
namespace process { class Launcher; }
namespace process { class User; }

namespace pdbuild {

class Options;

class LicenseAction {
private:
  LicenseAction();
    ~LicenseAction();

public:
    static int Run();
};

}

#endif // !pdbuild_LicenseAction_h
