//
//  Actions.c
//  xcdarwinbuild
//
//  Created by John Othwolo on 10/20/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//

#include <iostream>
#include <libutil/FSUtil.h>
#include <libutil/Filesystem.h>
#include <pdbuild/Action.h>
#include <pdbuild/Options.h>
#include <process/Context.h>

using pdbuild::Action;
using pdbuild::Options;
using libutil::Filesystem;
using libutil::FSUtil;

Action::
Action(){}

Action::
~Action(){}

//std::vector<pbxsetting::Level> Actions::
//CreateOverrideLevels(process::Context const *processContext, Filesystem const *filesystem, pbxsetting::Environment const &environment, Options const &options, std::string const &workingDirectory)
//{
//    std::vector<pbxsetting::Level> levels;
//
//    std::vector<pbxsetting::Setting> settings;
//    if (options.sdk()) {
//        settings.push_back(pbxsetting::Setting::Create("SDKROOT", *options.sdk()));
//    }
//    if (options.arch()) {
//        settings.push_back(pbxsetting::Setting::Create("ARCHS", *options.arch()));
//    }
//    levels.push_back(pbxsetting::Level(settings));
//
//    levels.push_back(options.settings());
//
//    if (options.xcconfig()) {
//        std::string path = FSUtil::ResolveRelativePath(*options.xcconfig(), workingDirectory);
//        ext::optional<pbxsetting::XC::Config> config = pbxsetting::XC::Config::Load(filesystem, environment, path);
//        if (!config) {
//            fprintf(stderr, "warning: unable to open xcconfig '%s'\n", options.xcconfig()->c_str());
//        } else {
//            levels.push_back(config->level());
//        }
//    }
//
//    if (ext::optional<std::string> configFile = processContext->environmentVariable("XCODE_XCCONFIG_FILE")) {
//        std::string path = FSUtil::ResolveRelativePath(*configFile, workingDirectory);
//        ext::optional<pbxsetting::XC::Config> config = pbxsetting::XC::Config::Load(filesystem, environment, path);
//        if (!config) {
//            fprintf(stderr, "warning: unable to open xcconfig from environment '%s'\n", path.c_str());
//        } else {
//            levels.push_back(config->level());
//        }
//    }
//
//    return levels;
//}

//xcexecution::Parameters Actions::
//CreateParameters(Options const &options, std::vector<pbxsetting::Level> const &overrideLevels)
//{
//    return xcexecution::Parameters(
//        options.workspace(),
//        options.project(),
//        options.scheme(),
//        (!options.target().empty() ? ext::make_optional(options.target()) : ext::nullopt),
//        options.allTargets(),
//        options.actions(),
//        options.configuration(),
//        overrideLevels);
//}

bool Action::
VerifyBuildActions(std::vector<std::string> const &actions)
{
    for (std::string const &action : actions) {
        if (action != "build" &&
            action != "build-for-testing" &&
            action != "analyze" &&
            action != "archive" &&
            action != "test" &&
            action != "test-without-building" &&
            action != "install-src" &&
            action != "install" &&
            action != "clean") {
            std::cerr << "error: unknown build action " << action << std::endl;
            return false;
        }
    }

    return true;
}

Action::Type Action::
Determine(Options const &options)
{
    if (options.version()) {
        return Version;
    } else if (options.usage()) {
        return Usage;
    } else if (options.help()) {
        return Help;
    } else if (options.license()) {
        return License;
    } else if (options.init()) {
        return Init;
    } else if (options.fetch()) {
        return Fetch;
    } else if (options.install()) {
        return Install;
    } else if (options.group()) {
        return Group;
    } else if (options.list()) {
        return List;
    } else if (options.globalCodesignIdentity()) {
        return SetGlobalCodeSignIdentity;
    } else {
        return Build;
    }
}
