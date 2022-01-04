//
//  InitAction.h
//  xcdarwinbuild
//
//  Created by John Othwolo on 10/21/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//

#ifndef pdbuild_InitAction_h
#define pdbuild_InitAction_h


namespace libutil { class Filesystem; }
namespace process { class PDBContext; }
namespace process { class Launcher; }
namespace process { class User; }

namespace pdbuild {

class Options;

class InitAction {
private:
    InitAction();
   ~InitAction();

public:
    static int
    Run(process::User const *user,
      process::PDBContext const *processContext,
      process::Launcher *processLauncher,
      libutil::Filesystem *filesystem, Options const &options);
};

}

#endif /* pdbuild_InitAction_h */
