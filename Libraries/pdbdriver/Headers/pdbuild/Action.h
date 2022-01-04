//
//  Actions.h
//  xcdarwinbuild
//
//  Created by John Othwolo on 10/20/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//
/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 * _
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef pdbuild_Action_h
#define pdbuild_Action_h


#include <string>
#include <vector>
#include <optional>

namespace libutil { class Filesystem; }
namespace process { class PDBContext; }

namespace pdbuild {

class Options;


class Action {
private:
    Action();
   ~Action();

public:
    /* The actions pdbuild can take. */
    enum Type {
        Build, // default action.
        Init,
        Fetch,
        Install, // install built project into DistributionRoot
        Group,
        List, // list projects
        SetGlobalCodeSignIdentity,
        Version,
        Usage,
        Help,
        License
    };

public:
    /*
     * Determine the action from a raw set of options. Picks the right
     * action even if multiple are specified or conflicting options are.
     */
    static Type
    Determine(Options const &options);

public:
    /*
     * Verifies that the passed in build actions are valid.
     */
    static bool
    VerifyBuildActions(std::vector<std::string> const &actions);

//public:
//    static std::vector<pbxsetting::Level>
//    CreateOverrideLevels(process::PDBContext const *processContext, libutil::Filesystem const *filesystem, pbxsetting::Environment const &environment, Options const &options, std::string const &workingDirectory);
//
//public:
//    static xcexecution::Parameters
//    CreateParameters(Options const &options, std::vector<pbxsetting::Level> const &overrideLevels);
};

}

#endif /* pdbuild_Action_h */
