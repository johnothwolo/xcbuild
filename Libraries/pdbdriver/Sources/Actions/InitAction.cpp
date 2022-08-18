/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <pdbuild/Actions/InitAction.h>
#include <pdbuild/Options.h>
#include <pdbuild/Usage.h>

#include <libutil/FSUtil.h>
#include <libutil/Filesystem.h>

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

#include <process/PDBContext.h>
#include <process/MemoryContext.h>
#include <process/Launcher.h>
#include <process/User.h>

#include <cerrno>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

using process::MemoryContext;
using pdbuild::InitAction;
using pdbuild::Options;
using libutil::Filesystem;
using libutil::FSUtil;

static const char* rootDirs[] = {
    ".pdbuild",
    "Logs",
    "Headers",
    "Roots",
    "SourceCache",
    "Symbols",
};

[[maybe_unused]] static const char *urlRegex_g =
"https?:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{2,256}\\.[a-z]{2,4}\b([-a-zA-Z0-9@:%_\\+.~#?&//=]*)";
[[maybe_unused]] static const char * sshRegex_g =
R"(/^[A-Za-z][A-Za-z0-9_]*\@[A-Za-z][A-Za-z0-9_\.]*\:(\/[A-Za-z][A-Za-z0-9_]*)*$/)";
//"/^[A-Za-z][A-Za-z0-9_]*\\@[A-Za-z][A-Za-z0-9_\\.]*\\:(\\/[A-Za-z][A-Za-z0-9_]*)*$/";

static constexpr char* dmgfile_name_g = (char*)"distribution_systemimage.sparsebundle";


static bool
createBuildRootDirectories(const std::string &buildroot, Filesystem *filesystem)
{
    for (auto &dir : rootDirs) {
        if (!filesystem->createDirectory(buildroot + "/" + dir, false)){
            return false;
        }
    }
    return true;
}

// pdb_distimage is the target dmg system image in the current PDBRoot.
// DistributionImage is a symlink to the above dmg's mountpoint.
static int createDistImage(process::Launcher *launcher,
                           const process::User *user,
                           const process::PDBContext *ctx,
                           Filesystem *filesystem,
                           const std::string &osBuild, bool withDmg) 
{
    
    if (withDmg) { // create DMG
        std::string volname = "DistributionImage_" + osBuild + std::string("-") + std::to_string(time(nullptr)/3600);
//        char temp[PATH_MAX];
        std::string targetSystemImage;
//        std::stringstream argStream;
        
        targetSystemImage = ctx->getPdBuildRootHiddenDirPath() + dmgfile_name_g;
        // check if there's an existing image.
        if (filesystem->exists(targetSystemImage)) {
          std::cout << "target image for " << osBuild << " already exists" << std::endl;
          return true;
        }

        // construct args
        std::vector<std::string> arguments = {
            "create",
            "-size", "100GB",
            "-fs", "HFSX",
            "-type", "SPARSEBUNDLE",
            "-quiet",
            "-uid", user->userID(),
            "-gid", user->groupID(),
            "-volname", volname,
            targetSystemImage,
        };

        ext::optional<std::string> executable = filesystem->findExecutable("hdiutil", ctx->executableSearchPaths());
        if (!executable) {
          // If hdiutil isn't available, fail.
          if (!executable)
            throw "error: could not find hdiutil in PATH\n";
        }
        // create dmg file
        MemoryContext hdiutil = process::MemoryContext(
            *executable,
            ctx->getPdBuildRootHiddenDirPath(),
            arguments,
            ctx->environmentVariables());
        ext::optional<int> exitCode = launcher->launch(filesystem, &hdiutil);

        if (!exitCode || *exitCode != 0) {
          throw "hdiutil Error.\n";
        }

        // create symlink
        return filesystem->writeSymbolicLink(
            std::string("/Volumes/").append(volname),
            ctx->getDestinationRootPath(), false);
    } else { // -nodmg
        std::string path = ctx->getDestinationRootPath();
        if (filesystem->exists(path)) {
          std::cout << "[Warning!]: DistributionImage directory already exists";
          return true;
        }
        return filesystem->createDirectory(path, true);
    }
}

plist::Object*
fetchInitPlistFile(const process::PDBContext *ctx,
                   const Filesystem *filesystem,
                   const std::string &plistPath)
{
  // if the path is empty return
  if (plistPath.empty()){
    std::cerr << "Bad plist path\n";
    return nullptr;
  }
  // implement remote file support later on.
  if (std::regex_match (plistPath, std::regex(urlRegex_g))) {
    std::cerr << "remote url plists not supported yet\n";
    return nullptr;
  } else if (std::regex_match (plistPath, std::regex(sshRegex_g))) {
    std::cerr << "remote ssh plists not supported yet\n";
    return nullptr;
  }
  // TODO: most likely a local file but we shouldn't assume that.
  else {
    if(!filesystem->exists(plistPath)) {
      std::cerr << "plist doesn't exist at given path\n";
      return nullptr;
    }
    ctx->openPlist(filesystem, plistPath);
    return ctx->getPlistRootObject();
  }
}

static std::unique_ptr<std::vector<uint8_t>>
packPlist(plist::Object *object){
  plist::Format::XML out = plist::Format::XML::Create( plist::Format::Encoding::UTF8);
  auto serialize = plist::Format::XML::Serialize(object, out);
  if (serialize.first == nullptr) {
    fprintf(stderr, "error: %s\n", serialize.second.c_str());
    return nullptr;
  }
  /* Write. */
  return std::move(serialize.first);
}

static bool savePlistFile(
    const process::PDBContext *ctx,
    libutil::Filesystem *filesystem,
    const std::string &plistDestPath,
    plist::Object *object){
  if (!ctx->Write(filesystem, *packPlist(object), plistDestPath)) {
    fprintf(stderr, "error: unable to write\n");
    return false;
  }
  return true;
}

static bool SetPlistFilePreference(const std::string &buildroot,
                                   const std::string &plFileName,
                                   const std::string &plChecksum){
  auto path = buildroot+"/.pdbuild/plist";
  std::ofstream file(path);
  file << plFileName;
  file.close();
  return true;
}

static bool SetBuildPreference(const std::string &buildroot,
                         const std::string &osBuild){
  auto path = buildroot+"/.pdbuild/build";
  std::ofstream file(path);
  file << osBuild;
  file.close();
  return true;
}

[[maybe_unused]]
static bool SetPlatformPreference(const std::string &buildroot,
                            const std::string &platform){
  return true;
}

[[maybe_unused]]
int createPDBDatabase(){
  return 0;
}

int
InitAction::Run(process::User const *user,
                process::PDBContext const *context,
                process::Launcher *processLauncher,
                Filesystem *filesystem, Options const &options)
{
    const auto& plistArg = options.init();
    const auto& path = context->executablePath();
    std::string osBuild;
    std::string arg;

    // get plist arg.
    arg = plistArg.value_or("help");
    if (arg == "help")
      std::cout << Usage::Text(FSUtil::GetBaseName(path), Usage::Option::InitHelp)
                << std::endl;

    // create directories if not created
    if(!createBuildRootDirectories(context->getPdBuildRoot(), filesystem)){
        if (errno == EEXIST)
            printf("Warning: pdbuild may already be initialized in this directory.\n");
        else {
          perror("Error: ");
          return errno;
        }
    }

    // get plist to extract build number
    auto root = fetchInitPlistFile(context, filesystem, *plistArg);
    if (root == nullptr)
      return 0;

    auto rootDict = plist::CastTo<plist::Dictionary>(root);

    osBuild = rootDict->value<plist::String>("build")->value();
//    std::cout << osBuild << std::endl;

    // create the dmg file or distribution directory
    if (!createDistImage(processLauncher, user, context, filesystem, osBuild, !options.noDmg()))
      return errno;

    // TODO: error checking
    SetBuildPreference(context->getPdBuildRoot(), osBuild);

    if (!savePlistFile(context, filesystem,context->getPdBuildRootHiddenDirPath()+FSUtil::GetBaseName(*plistArg), root))
    {
      std::cerr << "failed to save plist file\n";
      return 0;
    }

    // TODO: error checking
    SetPlistFilePreference(context->getPdBuildRoot(),
                           context->getPdBuildRootHiddenDirPath()+FSUtil::GetBaseName(*plistArg),
                           "");

    // we're almost done. create xref.db.
    // here are the tables:
    // projectfiles - file content for every project in the plist
    // distributionfiles - compiled executables, libraries, headers or resource files.
    // receipts - list of receipts and their project

    std::cout << "PDBROOT initialized\n";
    return 0;

//fail:
//    std::cout << Usage::Text(FSUtil::GetBaseName(path), Usage::Option::InitHelp)
//              << std::endl;
//out:
//    return 0;
}

InitAction:: InitAction() = default;
InitAction::~InitAction() = default;
