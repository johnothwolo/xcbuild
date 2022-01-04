//
//  Driver.cpp
//  xcdarwinbuild
//
//  Created by John Othwolo on 10/20/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//

#include <pdbuild/Action.h>
#include <pdbuild/Actions/BuildAction.h>
#include <pdbuild/Actions/CodesignAction.h>
#include <pdbuild/Actions/FetchAction.h>
#include <pdbuild/Actions/GroupAction.h>
#include <pdbuild/Actions/HelpAction.h>
#include <pdbuild/Actions/InitAction.h>
#include <pdbuild/Actions/InstallAction.h>
#include <pdbuild/Actions/LicenseAction.h>
#include <pdbuild/Actions/ListAction.h>
#include <pdbuild/Actions/UsageAction.h>
#include <pdbuild/Actions/VersionAction.h>
#include <pdbuild/Driver.h>
#include <pdbuild/Options.h>

#include <libutil/Filesystem.h>
#include <process/PDBContext.h>

#include <string>

using pdbuild::Driver;
using pdbuild::Action;
using pdbuild::Options;
using libutil::Filesystem;

Driver::Driver() = default;
Driver::~Driver() = default;

int Driver::
Run(process::User const *user, process::PDBContext const *processContext, process::Launcher *processLauncher, Filesystem *filesystem)
{
    Options options;
    auto &args = processContext->commandLineArguments();
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, args);
    if (!result.first) {
        fprintf(stderr, "error: %s\n\n", result.second.c_str());
        UsageAction::Run(processContext);
        return 1;
    }

    // current directory is buildroot by default.
    // Environment var PDB_BUILDROOT overrides default buildroot which is CWD/pwd.
    ((process::PDBContext*)processContext)->PDBRoot =
        processContext->environmentVariable("PDB_BUILDROOT").value_or(processContext->currentDirectory());
    
    Action::Type action = Action::Determine(options);
    
    switch (action) {
        case Action::Init:
            return InitAction::Run(user, processContext, processLauncher, filesystem, options);
        case Action::Version:
            return VersionAction::Run(user, processContext, filesystem, options);
        case Action::Usage:
            return UsageAction::Run(processContext);
        case Action::Help:
            return HelpAction::Run(processContext);
        case Action::License:
            return LicenseAction::Run();
        case Action::Fetch:
            return FetchAction::Run(user, processContext, processLauncher, filesystem, options);
        case Action::Install:
            return InstallAction::Run(user, processContext, processLauncher, filesystem, options);
        case Action::Group:
            return GroupAction::Run(user, processContext, processLauncher, filesystem, options);
        case Action::List:
            return ListAction::Run(user, processContext, processLauncher, filesystem, options);
        case Action::SetGlobalCodeSignIdentity:
            return CodesignAction::Run(user, processContext, processLauncher, filesystem, options);
        case Action::Build: /* FALLTHROUGH */
        default: // Build is the default action
            return BuildAction::Run(user, processContext, processLauncher, filesystem, options);
    }
}
