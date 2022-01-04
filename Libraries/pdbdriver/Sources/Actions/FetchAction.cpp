/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <pdbuild/Action.h>
#include <pdbuild/Actions/FetchAction.h>
#include <pdbuild/Options.h>
#include <pdbuild/Usage.h>

#include <libutil/Base.h>
#include <libutil/FSUtil.h>
#include <libutil/Filesystem.h>
#include <process/PDBContext.h>
#include <process/MemoryContext.h>
#include <process/Launcher.h>

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

#include <unistd.h>
#include <vector>
#include <cerrno>
#include <iostream>
#include <regex>
#include <sys/stat.h>
#include <dirent.h>

using pdbuild::FetchAction;
using pdbuild::Options;
using libutil::Filesystem;
using libutil::FSUtil;
using process::PDBContext;
using process::MemoryContext;

// static member data
const std::string FetchAction::SourceHost::Github = "github";
const std::string FetchAction::SourceHost::ArchiveUrl = "archive";
const std::string FetchAction::SourceHost::Apple = "apple";
const std::string FetchAction::SourceHost::Local = "local";

static int
DownloadGithubSource(
    const std::string &repo,
    const std::string &destArchiveName)
{
  return 0;
}

static int
DownloadAppleSource(
    const std::string &projectName,
    const std::string &projectVersion,
    const std::string &destArchiveName) {
  return 0;
}

static int
DownloadUrlSource(
    const std::string &src, const std::string &destArchiveName) {
  return 0;
}

static bool
DownloadLocalSource(
    const process::PDBContext *context,
    Filesystem *filesystem,
    const std::string &repo, // local directory holding the archive
    const std::string &destArchiveName,
    std::string &dstCacheFile) {
//  auto subst = repo.substr(0, 8);
//  if (subst != "file:///") context->abort("source_site mismatch");
//  std::string path = &repo.at(7);

  return filesystem->readDirectory(repo, false, [&filesystem, &repo, &dstCacheFile, &destArchiveName, &context](std::string const &name) {
    std::string withoutExtension = FSUtil::GetBaseNameWithoutExtension(FSUtil::GetBaseNameWithoutExtension(name));// for ".tar.gz", etc files
    std::string cacheFile = FSUtil::NormalizePath(context->PDBRoot+"/SourceCache/"+name);
    if (destArchiveName == withoutExtension) {
      if (filesystem->exists(dstCacheFile)) return true;
      if (!filesystem->copyFile(FSUtil::NormalizePath(repo + "/" + name), cacheFile))
        context->abort("Failed to copy file");
      dstCacheFile = cacheFile;
    }
    return true;
  });
}

[[maybe_unused]]
static int
ExtractSource(
    const process::PDBContext *ctx,
    process::Launcher *launcher,
    libutil::Filesystem *filesystem,
    const std::string &archiveSrc, const std::string &destDir)
{
  if(!filesystem->exists(archiveSrc))
    ctx->abort("\""+archiveSrc + "\" doesn't exist");

  // construct args
  std::string util = "(null)";
  std::vector<std::string> arguments;

  auto extension = FSUtil::GetFileExtension(archiveSrc);

  if(extension == "gz"){
    util = "tar";
    arguments.emplace_back("-xvf");
    arguments.push_back(archiveSrc);
    arguments.emplace_back("-C");
    arguments.push_back(destDir);
  } else if(extension == "tar.gz"){
    util = "tar";
    arguments.emplace_back("-xvzf");
    arguments.push_back(archiveSrc);
    arguments.emplace_back("-C");
    arguments.push_back(destDir);
  } else if (extension == "zip"){
    util = "unzip";
    arguments.push_back(archiveSrc);
    arguments.emplace_back("-d");
    arguments.push_back(destDir);
  } else {
    ctx->abort("Cannot handle source archive: "+FSUtil::GetBaseName(archiveSrc));
  }

  ext::optional<std::string> executable = filesystem->findExecutable(util, ctx->executableSearchPaths());
  if (!executable) {
    // If program isn't available, fail.
    if (!executable)
      ctx->abort("error: could not find "+util+" in PATH\n");
  }
  // create dmg file
  MemoryContext hdiutil = process::MemoryContext(
      *executable,
      ctx->currentDirectory(),
      arguments,
      ctx->environmentVariables());
  ext::optional<int> exitCode = launcher->launch(filesystem, &hdiutil);

  if (!exitCode || *exitCode != 0)
    ctx->abort(util+" Error: "+ strerror(errno));

  return 0;
}

static void
MountSystemImage(const process::PDBContext *ctx, process::Launcher *launcher, Filesystem *filesystem, const std::string &path){
  // construct args
  std::vector<std::string> arguments = {"attach", path};
  ext::optional<std::string> executable;

  // check if already mounted
  auto fpath = filesystem->readSymbolicLink(FSUtil::NormalizePath(ctx->PDBRoot+"/DistributionImage"));
  if(fpath != ext::nullopt)
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
}

// return negative number on error
int
FetchAction::Run(process::User const *user,
                 PDBContext const *processContext,
                 process::Launcher *processLauncher,
                 libutil::Filesystem *filesystem, Options const &options)
{
  if (!options.project())
    processContext->abort("Error: Project arg not provided");

  int ret = 0;
  auto plistRootDict = plist::CastTo<plist::Dictionary>(processContext->openPlist(filesystem).get());
  if (plistRootDict == nullptr) return -1;
  auto projectName = *options.project();
  auto sourceSites = plistRootDict->value<plist::Dictionary>("source_sites");
  auto allProjects = plistRootDict->value<plist::Dictionary>("projects");

  // if we want to fetch all just call ourselves recursively for every project.
  if(projectName == "all"){
    for(auto project = allProjects->begin(); project != allProjects->end();
         project++ ){
      ((Options&)options).setProject(*project);
      ret = FetchAction::Run(user, processContext, processLauncher, filesystem, options);
      if (ret != 0)
        return -1;
    }
  }

  auto projectDict = allProjects->value<plist::Dictionary>(projectName);
  auto projectVersion = options.projectVersion().value_or(projectDict->value<plist::String>("version")->value());
  auto projectNameVersionPrefix = projectName + "-" + projectVersion;

  if(std::find(projectDict->begin(), projectDict->end(), "original") != projectDict->end()){
    projectDict = allProjects->value<plist::Dictionary>(projectDict->value<plist::String>("original")->value());
  }

  auto projectSource = projectDict->value<plist::Dictionary>("source");
  auto host = projectSource->value<plist::String>("host")->value();
  std::string archivePath;

  if (host == SourceHost::Github) {
    auto repo = projectSource->value<plist::String>("repo")->value();
    ret = DownloadGithubSource(repo, projectNameVersionPrefix);
  } else if (host == SourceHost::ArchiveUrl){
    auto repo = projectSource->value<plist::String>("url")->value();
    ret = DownloadUrlSource(repo, projectNameVersionPrefix);
  } else if (host == SourceHost::Apple) {
    //    auto repo = projectSource->value<plist::String>("url")->value();
    ret = DownloadAppleSource(projectName, projectVersion, projectNameVersionPrefix);
  } else if (host == SourceHost::Local) {
    ret = !DownloadLocalSource(
        processContext, filesystem,
        sourceSites->value<plist::String>("local")->value(),
        projectNameVersionPrefix,
        archivePath);
  } else processContext->abort("Unsupported project source found");

  if (ret != 0)
    return ret;

  // if we reach here, the project archive is in source cache directory

  // check that the dmg is mounted.
  auto dmgFile = processContext->PDBRoot+"/.pdbuild/distribution_systemimage.sparsebundle";
  MountSystemImage(processContext, processLauncher, filesystem, dmgFile);
  // create relevant directories
  if(!filesystem->exists(processContext->PDBRoot+"/DistributionImage/Sources"))
    filesystem->createDirectory(processContext->PDBRoot+"/DistributionImage/Sources", false);

  // extract
  auto sourceDir = processContext->PDBRoot+"/DistributionImage/Sources";
  ExtractSource(processContext, processLauncher, filesystem, archivePath, sourceDir);

  // create receipts for source

  return ret;
}


// Download "$SourceCache" "$filename" "$($DARWINXREF source_sites $projnam)"
//static int DownloadSource(Filesystem *fs, std::string buildroot, std::string dest, std::string src){
//    auto fs = this->config->_fs;
//    auto filename = fname;
//
//    // check if the source was overidden in project
//    if (this->source.host == project_source_t::Apple){
//        // TODO: finish
//    } else if (this->source.host == project_source_t::Github){
//        // TODO: finish
//    } else if (this->source.host == project_source_t::Link){
//        // TODO: finish
//    } else if (this->source.host == project_source_t::Local){
//        // TODO: aren't we supposed to copy then extract?
//        auto rmi = std::any_cast<strpair_t>(this->source.remote_info);
//        auto link = rmi.second;
//        bool ret = true;
//
//        if (link == "inherit") {
//            link = config->source_sites.at(0);
//        }
//
//        link += "/" + this->name + "-" + this->version;
//
//        if (rmi.first == "Archive"){
//            ret = fs.copyFile(link, config->source_cache);
//        } else if (rmi.first == "directory"){
//            ret = fs.copyDirectory(link, config->source_cache+"/"+this->name+"-"+ this->version, true);
//        } else {
//            std::cerr << "Error." << std::endl;
//            exit(-1);
//        }
//        if(!ret){
//            std::cerr << "io operation failed." << std::endl;
//            exit(-1);
//        }
//    } else {
//        // use default source_sites
//        // TODO: ask user if they want to specify a custom url.
//        if(this->source.host == project_source_t::None){
//            auto sites = this->config->source_sites;
//            if (sites.size() < 1) {
//                std::cerr << "Cannot Download file for project" << this->name << std::endl;
//                exit(-1);
//            }
//
//            // TODO: site is most likely going to be opensource.apple.com, but this can be improved
//
//            for (auto &site : sites) {
//                filename += "-" + this->version + ".tar.gz";
//                DownloadFile(site+"/"+filename, config->source_cache+"/.tmp."+filename);
//            }
//        }
//    }
//
//
//
//    return true;
//
//}

pdbuild::FetchAction::FetchAction() = default;
pdbuild::FetchAction::~FetchAction() = default;
