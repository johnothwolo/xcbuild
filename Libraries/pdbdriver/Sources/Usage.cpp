/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <pdbuild/Usage.h>
#include <sstream>

#define nl      std::endl

using pdbuild::Usage;

std::string Usage::
Text(std::string const &name, Option type) {
    std::ostringstream result;
    
    result << "pdbuild 1.0" << nl; // TODO: Work on versioning
    result << "Copyright © 2021 The PureDarwin Project." << nl;
    result << "This is free software; see the source for copying conditions." << nl;
    result << nl;
    
    if(type == Option::InitHelp){
    result <<  "usage: " << name << " -init <build> [-nodmg] [help]" << nl;
    result <<  nl;
    result <<  "  <build>       can be a standard build number or a path to a plist." << nl;
    result <<  "                supported user@host:/dir/file.plist," << nl;
    result <<  "                          https://host/dir/file.plist," << nl;
    result <<  "                          paths: /dir/file.plist" << nl;
    result <<  "  -nodmg        do not use a sparse image for build root (use a regular directory)" << nl;
    return result.str();
    }
    
    
    result << "usage: " << name << " -init <build> [-nodmg]" << nl;
    result << "       " << name << " [action] [options] <Parameters> [<version>]" << nl;
    result << "  actions: [-help] [-headers] [-fetch] [-source] [-load] [-loadonly] [-group]" << nl;
    result << "  options: [-build <build>] [-target <target>]" << nl; // [-configuration=;
    result << "           [-logdeps] [-nopatch] [-noDeps] [-nosource] [-codesign <identity>]" << nl;
    result << "           [-depsbuild X [-depsbuild Y]]" << nl;
    result << nl;
    
    if (type == Option::Help) {
    result << "Initialize Build:" << nl;
    result << " -init                   Initialize a build (check '" << name << " "
                                                                "-init help' for more info)" << nl;
    result << nl;
    result << "Actions:" << nl;
//    result << " -build                  (Default) Build the given project (with dependencies) and install them into the DistributionRoot." << nl;
//    result << " -headers                Do the installhdrs phase, instead of install." << nl;
//    result << " -fetch                  Only download necessary source and patch files." << nl;
//    result << " -downloadOnly           Only download dependencies into the build root, but don't build." << nl;
    result << " --license               Print program license" << nl;
    result << " --version               Print program version and copyright" << nl;
    result << " -fetch                  Download, Extract, patch, and stage necessary project source and patch files." << nl;
    result << " -install                Install project into the DistributionRoot." << nl;
    result << " -group <group>          Build all projects in the given project group." << nl;
    result << " -list [build]           List all the projects for current build or for provided build number." << nl;
    result << " -setCodesign <identity> Set the global CodeSign to the given identity." << nl;
    result << nl;

    result << "Options:" << nl;
//    result << " -nosource               Do not fetch or stage source. This assumes that the." << nl;
//    result << "                         source is already in place in the BuildRoot." << nl;
//    result << " -nopatch                Don't patch sources before building." << nl;
    result << " -onlyDeps               Only perform action on the dependencies for this project." << nl;
    result << " -noInstall              Don't install project into the DistributionRoot." << nl;
    result << " -noDeps                 Don't handle dependencies for this project." << nl;
    result << " -target <target>        The makefile/xcode target to build." << nl;
    result << " -configuration <config> Specify the build configuration to use." << nl;
    result << " -build <build>          Specify the darwin build number to build, e.g. 8B15." << nl;
    result << " -version <version>      Specify the project version for the given project." << nl;
    result << " -codesign <identity>    Sign the project, using the given CODE_SIGN_IDENTITY value." << nl;
    result << nl;
    }
    
    result << " Parameters:" << nl;
    result << "  <project> The name of the project to build" << nl;
    result << "  <version> If specified, the version of the project to build" << nl;
    result << "            this will default to the version associated with the" << nl;
    result << "            currently running build." << nl;
    
    return result.str();
}
