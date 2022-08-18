/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

//#include <pdbuild/Action.h>
#include <pdbuild/Actions/BuildAction.h>
#include <pdbuild/Actions/FetchAction.h>
#include <pdbuild/Options.h>
#include <pdbuild/Project.h>
////#include <xcexecution/NinjaExecutor.h>
////#include <xcexecution/SimpleExecutor.h>
////#include <xcformatter/DefaultFormatter.h>
////#include <xcformatter/NullFormatter.h>
////#include <builtin/Registry.h>
//#include <libutil/Base.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <process/PDBContext.h>
#include <process/MemoryContext.h>
#include <process/Launcher.h>

#include <plist/Array.h>
#include <plist/Boolean.h>
//#include <plist/Data.h>
//#include <plist/Date.h>
#include <plist/Dictionary.h>
#include <plist/Integer.h>
//#include <plist/Real.h>
#include <plist/String.h>
#include <plist/Object.h>
//#include <plist/Format/Format.h>
//#include <plist/Format/Any.h>
//#include <plist/Format/XML.h>
//#include <plist/Format/JSON.h>

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unistd.h>

using pdbuild::BuildAction;
using pdbuild::Options;
using libutil::Filesystem;
using libutil::FSUtil;
using process::MemoryContext;
using BuildTool = pdbuild::BuildAction::BuildTool;

using pldictionary_t  = plist::Dictionary;
using plstring_t      = plist::String;
using plinteger_t     = plist::Integer;
using plboolean_t     = plist::Boolean;
template <typename T>

static inline T *pl_cast(plist::Object *obj) { return plist::CastTo<T>(obj); }

const std::string BuildTool::Invalid = "-";
const std::string BuildTool::Xcode = "xcodebuild";
const std::string BuildTool::Makefile = "make";
const std::string BuildTool::CMake = "cmake";
const std::string BuildTool::Ninja = "ninja";

BuildAction::BuildAction() = default;
BuildAction::~BuildAction() = default;

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

namespace std {
std::vector<std::string> string_split(const std::string &s, char delim) {
  std::vector<std::string> result;
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim)) {
      result.push_back(item);
  }
  return result;
}

// from stack overflow
void string_replace(std::string& str, const std::string& from, const std::string& to, bool allOccurrences) {
    if(from.empty())
        return;
    size_t start_pos = 0;

    // if we're not replacing all occurrences
    if (allOccurrences == false){
      start_pos = str.find(from);
      if(start_pos == std::string::npos) return;
      str.replace(start_pos, from.length(), to);
    } else while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    
}

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

// appends xcode build settings for xcode project types
static void appendXcodeBuildSettings(
    const process::PDBContext *ctx,
    pdbuild::Project &project,
    std::vector<std::string> &arguments,
    std::unordered_map<std::string, std::string> &envars // to replace '$(SRCROOT)' and friends with the actual paths
){
  auto settings = project.getXcodeBuildSettings();
  auto projectType = project.getXcodeProductBundleType();

  // add a trailing space if there's already stuff in these buildsettings
  if (!settings["HEADER_SEARCH_PATHS"].empty())
     settings["HEADER_SEARCH_PATHS"] += " ";
  if (!settings["FRAMEWORK_SEARCH_PATHS"].empty())
     settings["FRAMEWORK_SEARCH_PATHS"] += " ";
  if (!settings["LIBRARY_SEARCH_PATHS"].empty())
     settings["LIBRARY_SEARCH_PATHS"] += " ";

  // if project is kext, ommit some incldue paths. only include kernel frameworks... and /usr/local/include (but at the end; order matters)
  if (projectType == "KEXT"){
    settings["HEADER_SEARCH_PATHS"] +=     ctx->getDestinationRootPath()+"System/Library/Frameworks/Kernel.framework/Versions/A/PrivateHeaders";
    settings["HEADER_SEARCH_PATHS"] += " "+ctx->getDestinationRootPath()+"System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders";
    // settings["HEADER_SEARCH_PATHS"] += " "+ctx->getDestinationRootPath()+"usr/local/include"; // for xxx_private.h headers
  } else {
    settings["HEADER_SEARCH_PATHS"] +=     ctx->getDestinationRootPath()+"System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders"; // does this go first?
    settings["HEADER_SEARCH_PATHS"] += " "+ctx->getDestinationRootPath()+"usr/include";
    settings["HEADER_SEARCH_PATHS"] += " "+ctx->getDestinationRootPath()+"usr/local/include"; // for xxx_private.h headers

    settings["FRAMEWORK_SEARCH_PATHS"] +=     ctx->getDestinationRootPath()+"System/Library/Frameworks";
    settings["FRAMEWORK_SEARCH_PATHS"] += " "+ctx->getDestinationRootPath()+"System/Library/PrivateFrameworks";

    settings["LIBRARY_SEARCH_PATHS"] +=     ctx->getDestinationRootPath()+"usr/lib";
    settings["LIBRARY_SEARCH_PATHS"] += " "+ctx->getDestinationRootPath()+"usr/local/lib";

    // settings["HEADER_SEARCH_PATHS"] += " $(DSTROOT)/../../../usr/include  $(DSTROOT)/../../../usr/local/include";
  }



//  /Volumes/DistributionImage_PD18_0-455927/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
  // append our Xcode build setting with dependencies from the distribution image
  // add leading space to variable just in case it's set to something
  // settings["HEADER_SEARCH_PATHS"] += " $(DSTROOT)/../../../usr/include  $(DSTROOT)/../../../usr/local/include";
  // settings["FRAMEWORK_SEARCH_PATHS"] += " $(DSTROOT)/../../../System/Library/Frameworks $(DSTROOT)/../../../System/Library/PrivateFrameworks";
  // settings["LIBRARY_SEARCH_PATHS"] += " $(DSTROOT)/../../../usr/lib $(DSTROOT)/../../../usr/local/lib";

  for (auto &setting : settings){

    // replace '$(SRCROOT)' and friends with the actual paths because xcodebuild keeps adding double quotes
    // apparently double quotes causes the '$' to be parsed as a command and not a variable.
    std::string_replace(setting.second, "$(SRCROOT)", envars["SRCROOT"], true);
    std::string_replace(setting.second, "$(OBJROOT)", envars["OBJROOT"], true);
    std::string_replace(setting.second, "$(SYMROOT)", envars["SYMROOT"], true);
    std::string_replace(setting.second, "$(DSTROOT)", envars["DSTROOT"], true);

    // arguments.push_back(setting.first+"=$("+setting.first+") "+setting.second+" ");
    arguments.emplace_back(setting.first+"="+setting.second);

    // auto splitBuildSettings = std::string_split(setting.second, ' ');
    // std::vector<std::string> finalArgs = {setting.first+"='"};
    // finalArgs.insert(finalArgs.end(), splitBuildSettings.begin(), splitBuildSettings.end());
    // finalArgs.back() += "'"; // add single quote to last string
    // arguments.insert(arguments.end(), finalArgs.begin(), finalArgs.end()); // insert args into main args
  }
}

static void setIntermediateDirectories(
    const process::PDBContext *context,
    Filesystem *filesystem,
    std::unordered_map<std::string, std::string> &env,
    const std::string &projectName,
    const std::string &projectDir
){
  auto projectDirName = FSUtil::GetBaseName(projectDir);
  auto tempDir = context->getDestinationTmpPath();
  auto projectIntermediateDir = tempDir+"/"+projectName;
  std::string tmp_path;

  // create /private/var/tmp/<project>
  if (!filesystem->createDirectory(projectIntermediateDir, true) && errno != EEXIST)
    context->abort("unable to create directory");

  // create and set ".obj" dir
  tmp_path = projectIntermediateDir+"/"+projectDirName+".obj";
  if(!filesystem->createDirectory(tmp_path, true) && errno != EEXIST)
    context->abort("unable to create directory");
  env["OBJROOT"] = tmp_path;

  // create and set ".sym" dir
  tmp_path = projectIntermediateDir+"/"+projectDirName+".sym";
  if(!filesystem->createDirectory(tmp_path, true) && errno != EEXIST)
    context->abort("unable to create directory");
  env["SYMROOT"] = tmp_path;

  // create and set ".root" dir
  tmp_path = projectIntermediateDir+"/"+projectDirName+".root";
  if(!filesystem->createDirectory(tmp_path, true) && errno != EEXIST)
    context->abort("unable to create directory");
  env["DSTROOT"] = tmp_path;
}

static void
MountSystemImage(const process::PDBContext *ctx, process::Launcher *launcher, Filesystem *filesystem, const std::string &path){
  // construct args
  std::vector<std::string> arguments = {"attach", path};
  ext::optional<std::string> executable;

  // check if already mounted
  auto tmpStr = filesystem->resolvePath(ctx->getDestinationRootPath());
  if(!tmpStr.empty())
    return;

  // search for hdiutil
  executable = filesystem->findExecutable("hdiutil", ctx->executableSearchPaths());
  if (!executable) {
    // If hdiutil isn't available, fail.
    if (!executable)
      ctx->abort("Error: could not find hdiutil in PATH");
  }

  // mount dmg file
  MemoryContext hdiutil = process::MemoryContext(
      *executable,
      ctx->currentDirectory(),
      arguments,
      ctx->environmentVariables());
  ext::optional<int> exitCode = launcher->launch(filesystem, &hdiutil);

  if (!exitCode || *exitCode != 0)
    ctx->abort("hdiutil Error.");

  auto sources =ctx->getDestinationSourcesPath();
  auto receipts = ctx->getDestinationReceiptsPath();
  auto varTmp = ctx->getDestinationVarTmpPath();
  auto tmp = ctx->getDestinationTmpPath();

  if(!filesystem->exists(sources)) filesystem->createDirectory(sources, false);
  if(!filesystem->exists(receipts)) filesystem->createDirectory(receipts, true);
  if(!filesystem->exists(varTmp)) filesystem->createDirectory(varTmp, true);
  if(!filesystem->exists(tmp) || filesystem->readSymbolicLink(tmp) == ext::nullopt)
    filesystem->writeSymbolicLink(varTmp, tmp, true);
}

static int
RunBuildTool(
    const process::PDBContext *ctx,
    process::Launcher *launcher,
    Filesystem *filesystem,
    const std::string &sdk,
    const std::string &buildTool,
    const std::string &projectDir,
    std::string &dstRoot,
    pdbuild::Project &project)
{
  // search for the buildTool
  std::vector<std::string> arguments{};
  std::unordered_map<std::string, std::string>
      envars(ctx->environmentVariables().begin(), ctx->environmentVariables().end());
  ext::optional<std::string> executable;

//  envars["DEVELOPER_DIR"] =
//      ctx->environmentVariable("DEVELOPER_DIR").value_or(FSUtil::NormalizePath(sdk+"/Developer"));
  envars["SRCROOT"] = projectDir;
  envars["PWD"] = projectDir;

//  executable = envars["DEVELOPER_DIR"] + "/usr/bin/" + buildTool;
  executable = filesystem->findExecutable(buildTool, ctx->executableSearchPaths());

  if (!executable) {
    // If the buildTool isn't available, fail.
    if (!executable)
      ctx->abort("Error: could not find "+buildTool+" in PATH");
  }

  // should 4th arg be targetName or projectName?
  setIntermediateDirectories(ctx, filesystem, envars,
                             project.isAlias() ? project.getOriginalProjectName() : project.getProjectName(), projectDir);

  // NOTE: target has to be specified if the project doesn't have a single target.
  auto target = project.getProjectTarget();
  // get version to use for building sdk
  auto macosxVersion = ctx->getPlistRootDictionary()->value<plist::String>("macosx");

  if (buildTool == BuildTool::Xcode) {
    arguments.emplace_back("-sdk");
    arguments.push_back(macosxVersion->value());
    arguments.emplace_back("install");
    if (!target.empty()) {
      arguments.emplace_back("-target");
      arguments.push_back(target);
    }
    arguments.push_back("SRCROOT="+envars["SRCROOT"]);
    arguments.push_back("OBJROOT="+envars["OBJROOT"]);
    arguments.push_back("SYMROOT="+envars["SYMROOT"]);
    arguments.push_back("DSTROOT="+envars["DSTROOT"]);

    // add build settings if they exist. also an opportunity to add our build settings
    appendXcodeBuildSettings(ctx, project, arguments, envars);

    // std::cout << "TESTING-:" << headerSearchPaths.str() << std::endl;
  } else if (buildTool == BuildTool::Makefile) {
    if (!target.empty())
      arguments.push_back(target);
    arguments.push_back("SDKROOT="+macosxVersion->value());
    arguments.push_back("SRCROOT="+envars["SRCROOT"]);
    arguments.push_back("OBJROOT="+envars["OBJROOT"]);
    arguments.push_back("SYMROOT="+envars["SYMROOT"]);
    arguments.push_back("DSTROOT="+envars["DSTROOT"]);
  } else if (buildTool == BuildTool::CMake) {
    arguments.push_back("-DCMAKE_INSTALL_PREFIX="+envars["DSTROOT"]);
    arguments.push_back("--build");
    arguments.push_back(projectDir);
    arguments.emplace_back("--target");
    arguments.push_back(target);
    arguments.emplace_back("--config", "Debug");
  }

  // add project specific envs as args...
  auto projectEnv = project.getEnvironmentVariables();
  for(auto &env: projectEnv){
    arguments.emplace_back(env.first+"="+env.second);
  }

  // FIXME: why tf are there quotes in my arguments?????
  // for (auto arg : arguments){
  //   std::cout << arg << std::endl;
  // }

  // run build tool
  MemoryContext buildToolContext(
      *executable,
      projectDir,
      arguments,
      envars
      );

  ext::optional<int> exitCode = launcher->launch(filesystem, &buildToolContext);

  if (!exitCode || *exitCode != 0)
    ctx->abort(buildTool+" error.");

  // Set dstRoot to DSTROOT
  dstRoot = envars["DSTROOT"];
  return *exitCode;
}

static int InstallDstRoot(const process::PDBContext *ctx, process::Launcher *launcher, Filesystem *filesystem, const std::string &dstRoot){
  // construct args
  std::vector<std::string> arguments = {dstRoot, ctx->getDestinationRootPath()};
  ext::optional<std::string> executable;

  // std::cout << "Copying \"" << dstRoot << "\" to \"" << ctx->getDestinationRootPath() << "\""<< std::endl;
  // search for ditto
  executable = filesystem->findExecutable("ditto", ctx->executableSearchPaths());
  if (!executable) {
    // If ditto isn't available, fail.
    if (!executable)
      ctx->abort("Error: could not find ditto in PATH");
  }

  // copy project root into Destination
  MemoryContext ditto = process::MemoryContext(
      *executable,
      ctx->currentDirectory(),
      arguments,
      ctx->environmentVariables());
  ext::optional<int> exitCode = launcher->launch(filesystem, &ditto);

  if (!exitCode || *exitCode != 0)
    ctx->abort("ditto Error.");

  return *exitCode;
}

int BuildAction::
Run(process::User const *user, process::PDBContext const *processContext, process::Launcher *processLauncher, Filesystem *filesystem, Options const &options)
{
  int ret = 0;
  ext::optional<std::string> pdsdk;
  std::string buildTool = BuildTool::Invalid;
  std::string sourcePath, dstRoot;

  // verify proj name
  if(!options.project())
    processContext->abort("Invalid project name");

  // open plist
  processContext->openPlist(filesystem);

  pdbuild::Project project(processContext, options);

  // get source directory full path
  sourcePath = processContext->getDestinationSourcesPath()+project.getProjectNameAndVersion();

  // download and extract source if it doesn't exist
  if(!filesystem->exists(sourcePath)) {
    ret = pdbuild::FetchAction::Run(user, processContext, processLauncher, filesystem, options);
    if (ret != 0)
      return ret;
  }

  // check what type of build system this project uses
  filesystem->readDirectory(sourcePath, false, [&buildTool](const std::string &name){
    auto extension = FSUtil::GetFileExtension(name);
    // only set once, in case we come across 2 or all types
    if (buildTool == BuildTool::Invalid) {
      // this is the preference order. xcode is always preferred above others.
      if (extension == "xcodeproj") {
        buildTool = BuildTool::Xcode;
      } else if (name == "Makefile") {
        buildTool = BuildTool::Makefile;
      } else if (name == "CMakelists.txt") { // we prefer makefile to cmake
        buildTool = BuildTool::CMake;
      }
    }
    return true;
  });

  // fail if we don't recognize the project type
  if(buildTool == BuildTool::Invalid) {
    processContext->abort("Cannot build this project buildTool");
  }

  // make sure image is mounted
  MountSystemImage(processContext, processLauncher, filesystem, processContext->getDestinationImagePath());

  // make sure PDBUILD_SDK var is set to a valid PureDarwin SDK.
//  pdsdk = processContext->environmentVariable("PDBUILD_SDK");
//  if (!pdsdk){
//    processContext->abort("Invalid SDK path");
//  }

  // check for dependencies and run them
  if(project.hasDependencies()){
    auto headerDeps = project.getHeaderDependencies();
    auto buildDeps = project.getBuildDependencies();
    auto customOptions = Options(options);
    if (headerDeps){
      for (auto hdep = headerDeps->begin(); hdep < headerDeps->end(); hdep++) {
        customOptions.setProject(pl_cast<plstring_t>(hdep->get())->value());
        ret = BuildAction::Run(user, processContext, processLauncher, filesystem, customOptions);
        if (ret != 0)
          return ret;
      }
    } else if (buildDeps){
      for (auto bdep = buildDeps->begin(); bdep < buildDeps->end(); bdep++) {
        customOptions.setProject(pl_cast<plstring_t>(bdep->get())->value());
        ret = BuildAction::Run(user, processContext, processLauncher, filesystem, customOptions);
        if (ret != 0)
          return ret;
      }
    }
  }

  std::cout << "--------------------------------------------------------------" << std::endl;
  std::cout << "---------------- BUILDING: " << project.getProjectName() <<  std::endl;
  std::cout << "--------------------------------------------------------------" << std::endl;

  // run buildtool.
  ret = RunBuildTool(processContext, processLauncher, filesystem, *pdsdk, buildTool, sourcePath, dstRoot, project);
  if (ret != 0) goto ERROR_OUT;

  std::cout <<  std::endl;
  std::cout << "Build Complete." << std::endl;
  std::cout << "Installing project root to target..." << std::endl;

  // install project from DSTROOT into destination.
  ret = InstallDstRoot(processContext, processLauncher, filesystem, dstRoot);
  if(ret != 0) goto ERROR_OUT;

  std::cout << "Done." << std::endl;
  std::cout << "--------------------------------------------------------------" << std::endl;
  return ret;
ERROR_OUT:
  std::cout << "Error!" << std::endl;
  std::cout << "--------------------------------------------------------------" << std::endl;
  return -1;
}
