/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <pdbuild/Action.h>
#include <pdbuild/Actions/BuildAction.h>
#include <pdbuild/Actions/FetchAction.h>
#include <pdbuild/Options.h>
//#include <xcexecution/NinjaExecutor.h>
//#include <xcexecution/SimpleExecutor.h>
//#include <xcformatter/DefaultFormatter.h>
//#include <xcformatter/NullFormatter.h>
//#include <builtin/Registry.h>
#include <libutil/Base.h>
#include <libutil/Filesystem.h>
#include <process/PDBContext.h>

#include <plist/Array.h>
#include <plist/Boolean.h>
#include <plist/Data.h>
#include <plist/Date.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
#include <plist/Real.h>
#include <plist/String.h>
#include <plist/Object.h>
#include <plist/Format/Format.h>
#include <plist/Format/Any.h>
#include <plist/Format/XML.h>
#include <plist/Format/JSON.h>

#include <iostream>
#include <unistd.h>

using pdbuild::BuildAction;
using pdbuild::Options;
using libutil::Filesystem;

BuildAction::
BuildAction(){}

BuildAction::
~BuildAction(){}

[[maybe_unused]]
static std::vector<std::string> verifyProjectRoot(process::PDBContext *processContext, Filesystem *filesystem){
    std::vector<std::string> result;
    std::string pwd = processContext->currentDirectory();
    
    
//    for (auto &dir : rootDirs) {
//        auto path = pwd.append("/").append(dir);
//        if (!filesystem->exists(path))
//            result.push_back(path);
//    }
    
    return result;
}


//static std::shared_ptr<xcformatter::Formatter>
//CreateFormatter(ext::optional<std::string> const &formatter)
//{
//    if (!formatter || *formatter == "default") {
//        /* Only use color if attached to a terminal. */
//        bool color = isatty(fileno(stdout));
//        auto formatter = xcformatter::DefaultFormatter::Create(color);
//        return std::static_pointer_cast<xcformatter::Formatter>(formatter);
//    } else if (*formatter == "null") {
//        auto formatter = xcformatter::NullFormatter::Create();
//        return std::static_pointer_cast<xcformatter::Formatter>(formatter);
//    }
//
//    return nullptr;
//}

//static std::unique_ptr<xcexecution::Executor>
//CreateExecutor(
//    ext::optional<std::string> const &executor,
//    std::shared_ptr<xcformatter::Formatter> const &formatter,
//    bool dryRun,
//    bool generate)
//{
//    if (!executor || *executor == "simple") {
//        auto registry = builtin::Registry::Default();
//        auto executor = xcexecution::SimpleExecutor::Create(formatter, dryRun, registry);
//        return libutil::static_unique_pointer_cast<xcexecution::Executor>(std::move(executor));
//    } else if (*executor == "ninja") {
//        auto executor = xcexecution::NinjaExecutor::Create(formatter, dryRun, generate);
//        return libutil::static_unique_pointer_cast<xcexecution::Executor>(std::move(executor));
//    }
//
//    return nullptr;
//}

//static bool
//VerifySupportedOptions(Options const &options)
//{
//    if (options.toolchain()) {
//        fprintf(stderr, "warning: toolchain option not implemented\n");
//    }
//
//    if (options.quiet() || options.verbose() || options.json() || options.hideShellScriptEnvironment()) {
//        fprintf(stderr, "warning: output options not implemented\n");
//    }
//
//    if (options.destination() || options.destinationTimeout()) {
//        fprintf(stderr, "warning: destination option not implemented\n");
//    }
//
//    if (options.parallelizeTargets() || options.jobs()) {
//        fprintf(stderr, "warning: job control option not implemented\n");
//    }
//
//    if (options.enableAddressSanitizer() || options.enableThreadSanitizer() || options.enableCodeCoverage()) {
//        fprintf(stderr, "warning: build mode option not implemented\n");
//    }
//
//    if (options.derivedDataPath()) {
//        fprintf(stderr, "warning: custom derived data path not implemented\n");
//    }
//
//    if (options.resultBundlePath()) {
//        fprintf(stderr, "warning: result bundle path not implemented\n");
//    }
//
//    if (options.xctestrun() || !options.onlyTesting().empty() || !options.skipTesting().empty()) {
//        fprintf(stderr, "warning: testing options not implemented\n");
//    }
//
//    for (std::string const &action : options.actions()) {
//        if (action != "build") {
//            fprintf(stderr, "warning: non-build action %s not implemented\n", action.c_str());
//        }
//    }
//
//    return true;
//}

int BuildAction::
Run(process::User const *user, process::PDBContext const *processContext, process::Launcher *processLauncher, Filesystem *filesystem, Options const &options)
{
  int ret = 0;
  auto root = processContext->openPlist(filesystem);

  if (root == nullptr)
    return ret;

  auto rootDict = plist::CastTo<plist::Dictionary>(root.get());
  auto projectDict = rootDict->value<plist::Dictionary>(
                              "projects")->value<plist::Dictionary>(*options.project());
  auto projectVersion = options.projectVersion().value_or(projectDict->value<plist::String>("version")->value());
  auto osBuild = rootDict->value<plist::String>("build")->value();
  auto sourceDirName = *options.project() + "-" + projectVersion;

  // download and extrace source if it doesn't exist
  if(!filesystem->exists(processContext->PDBRoot+"/SourceCache/"+sourceDirName)) {
    ret = pdbuild::FetchAction::Run(user, processContext, processLauncher, filesystem,
                              options);
    if (ret != 0)
      return ret;
  }


  std::cout << "Project: " << std::endl;

  return ret;
//    // TODO(grp): Implement these options.
//    if (!VerifySupportedOptions(options)) {
//        return -1;
//    }
//
//    /* Verify the build options are not conflicting or invalid. */
//    if (!Actions::VerifyBuildActions(options.actions())) {
//        return -1;
//    }

    /*
     * Create the formatter to format the build log.
     */
//    std::shared_ptr<xcformatter::Formatter> formatter = CreateFormatter(options.formatter());
//    if (formatter == nullptr) {
//        fprintf(stderr, "error: unknown formatter '%s'\n", options.formatter()->c_str());
//        return -1;
//    }

    /*
     * Create the executor used to perform the build.
     */
//    std::unique_ptr<xcexecution::Executor> executor = CreateExecutor(options.executor(), formatter, options.dryRun(), options.generate());
//    if (executor == nullptr) {
//        fprintf(stderr, "error: unknown executor '%s'\n", options.executor()->c_str());
//        return -1;
//    }

    /*
     * Use the default build environment. We don't need anything custom here.
     */
//    ext::optional<pbxbuild::Build::Environment> buildEnvironment = pbxbuild::Build::Environment::Default(user, processContext, filesystem);
//    if (!buildEnvironment) {
//        fprintf(stderr, "error: couldn't create build environment\n");
//        return -1;
//    }

    /* The build settings passed in on the command line override all others. */
//    std::vector<pbxsetting::Level> overrideLevels = Actions::CreateOverrideLevels(
//        processContext,
//        filesystem,
//        buildEnvironment->baseEnvironment(),
//        options,
//        processContext->currentDirectory());

    /*
     * Create the build parameters. The executor uses this to load a workspace and create a
     * build context, but is not required to when the parameters haven't changed from a cache.
     */
//    xcexecution::Parameters parameters = Actions::CreateParameters(options, overrideLevels);

    /*
     * Perform the build!
     */
//    bool success = executor->build(user, processContext, processLauncher, filesystem, *buildEnvironment, parameters);
//    if (!success) {
//        return 1;
//    }

    return 0;
}
