/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <pdbuild/Actions/FetchAction.h>
#include <pdbuild/Options.h>
#include <pdbuild/Project.h>

#include <libutil/FSUtil.h>
#include <libutil/DefaultFilesystem.h>
#include <process/PDBContext.h>
#include <process/MemoryContext.h>
#include <process/Launcher.h>

#include <plist/Array.h>
#include <plist/Dictionary.h>
#include <plist/String.h>
#include <plist/Object.h>
#include <vector>
#include <cerrno>

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

using SourceHost = pdbuild::FetchAction::SourceHost;


// Only remote source check archive. Maybe switch Local source to archive-only?
[[maybe_unused]]
static bool CheckProjectArchive(const std::string& projectName, process::PDBContext *processContext, Filesystem *filesystem) {
  // check if an archive containing project name exists.
  return filesystem->readDirectory(processContext->getSourceCachePath(), false, [&projectName](const std::string &name){
    auto found = name.find('.');
    auto filename = name.substr(0, found == std::string::npos ? name.size(): found - 1);
    // only set once, in case we come across 2 or all types
    if (filename == projectName){
      return true;
    }
    return false;
  });
}

static int
DownloadGithubSource(
    const std::string &repo,
    const std::string &destArchiveName)
{
  return 0;
}

static int
DownloadAppleSource(
    pdbuild::Project *project
) {
  std::string projectName;
  std::string projectVersion;
  std::string destArchiveName;
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
    pdbuild::Project *project,
    std::string &dstCacheFile) {
  std::string destArchiveName = project->getProjectNameAndVersion();
//  auto subst = repo.substr(0, 8);
//  if (subst != "file:///") context->abort("source_site mismatch");
//  std::string path = &repo.at(7);
  std::string path = repo;

  return filesystem->readDirectory(path, false, [&filesystem, &path, &dstCacheFile, &destArchiveName, &context](std::string const &name) {
    long dot = -1;
    if((dot = (long)name.find(".tar.gz")) >= 0 ||
        (dot = (long)name.find(".tar.xz")) >= 0 ||
        (dot = (long)name.find(".tar.bz2")) >= 0 ||
        (dot = (long)name.find(".bz2")) >= 0 ||
        (dot = (long)name.find(".xz")) >= 0 ||
        (dot = (long)name.find(".gz")) >= 0 ||
        (dot = (long)name.find(".7z")) >= 0 ||
        (dot = (long)name.find(".zip")) >= 0)
    {
      std::string withoutExtension = name.substr(0, dot == std::string::npos ? name.size() : dot);// for ".tar.gz", etc files
      std::string cacheFile = FSUtil::NormalizePath(context->getSourceCachePath() + name);
      if (destArchiveName == withoutExtension) {
        if (filesystem->exists(cacheFile)) {
          dstCacheFile = cacheFile;
          return true; // if source exists, don't copy anything
        }
        if (!filesystem->copyFile(FSUtil::NormalizePath(path + "/" + name), cacheFile))
          context->abort("Failed to copy file");
        dstCacheFile = cacheFile;
        return true;
      }
    }
    return false; // if we find nothing return false
  });
}

static int
ProcessSourceSite(
    const process::PDBContext *context,
    Filesystem *filesystem,
    plist::Dictionary *sourceSite,
    pdbuild::Project *project,
    std::string &archivePath)
{
  // check the host, then download
  auto host = sourceSite->value<plist::String>("host");
  if (host == nullptr)
    context->abort("Unsupported project source");

  if (host->value() == SourceHost::Github) {
    auto repo = sourceSite->value<plist::String>("repo")->value();
    return DownloadGithubSource(repo, project->getProjectNameAndVersion());
  } else if (host->value() == SourceHost::ArchiveUrl) {
    auto repo = sourceSite->value<plist::String>("url")->value();
    return DownloadUrlSource(repo, project->getProjectNameAndVersion());
  } else if (host->value() == SourceHost::Apple) {
    return DownloadAppleSource(project);
  } else if (host->value() == SourceHost::Local) {
    auto repo = sourceSite->value<plist::String>("url")->value();
    return !DownloadLocalSource(context, filesystem, repo, project, archivePath);
  } else return -1;
}


[[maybe_unused]]
static int
ExtractSource(
    const process::PDBContext *ctx,
    process::Launcher *launcher,
    libutil::Filesystem *filesystem,
    const std::string &archiveSrc,
    const std::string &projectDirName)
{
  auto destDir = ctx->getDestinationSourcesPath();
  if(!filesystem->exists(archiveSrc))
    ctx->abort("\""+archiveSrc + "\" doesn't exist");

  if(filesystem->exists(destDir+"/"+projectDirName))
    return 0;

  // construct args
  std::string archiveUtil = "(null)";
  std::vector<std::string> arguments;

  if ((long)archiveSrc.find(".tar") >= 0){
    archiveUtil = "tar";
    if ((long)archiveSrc.find(".gz") >= 0) {
GZ:  arguments.emplace_back("-xzf");
    } else if ((long)archiveSrc.find(".bz2") >= 0) {
BZ2:   arguments.emplace_back("-xyf");
    } else if ((long)archiveSrc.find(".xz") >= 0) {
XZ:   arguments.emplace_back("-xJf");
    }
    arguments.push_back(archiveSrc);
    arguments.emplace_back("-C");
    arguments.push_back(destDir);
  } else if ((long)archiveSrc.find(".bz2") >= 0){
    goto BZ2;
  } else if ((long)archiveSrc.find(".xz") >= 0){
    goto XZ;
  } else if ((long)archiveSrc.find(".gz") >= 0){
    goto GZ;
  } /* else if ((extensionDot = (long)archiveSrc.find(".7z")) >= 0){

  } */
  else if ((long)archiveSrc.find(".zip") >= 0){
    archiveUtil = "unzip";
    arguments.push_back(archiveSrc);
    arguments.emplace_back("-d");
    arguments.push_back(destDir);
  } else {
    ctx->abort("Cannot handle source archive: "+FSUtil::GetBaseName(archiveSrc));
  }

  ext::optional<std::string> executable = filesystem->findExecutable(archiveUtil, ctx->executableSearchPaths());
  if (!executable) {
    // If program isn't available, fail.
    if (!executable)
      ctx->abort("error: could not find "+ archiveUtil +" in PATH\n");
  }

  // extract archive file
  MemoryContext archiveUtilContext = process::MemoryContext(
      *executable,
      ctx->currentDirectory(),
      arguments,
      ctx->environmentVariables());
  ext::optional<int> exitCode = launcher->launch(filesystem, &archiveUtilContext);

  if (!exitCode || *exitCode != 0)
    ctx->abort(archiveUtil +" Error: "+ strerror(errno));

  return 0;
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
}

// return negative number on error
int
FetchAction::Run(process::User const *user,
                 PDBContext const *processContext,
                 process::Launcher *processLauncher,
                 libutil::Filesystem *filesystem, Options const &options)
{
  int ret = 0;
  plist::Dictionary *allProjects;
  plist::Array *globalSourceSites;
  plist::Dictionary *projectSourceSiteOverride;
  std::string projectNameOption;
  std::string sourceDir;
  std::string archivePath;

  // is there a need to check if project arg is okay?
  if (!options.project())
    processContext->abort("Error: Project arg not provided");

  // open plist file
  processContext->openPlist(filesystem);

  projectNameOption = *options.project();
  globalSourceSites = processContext->getSourceSites();
  allProjects = processContext->getPlistRootDictionary()->value<plist::Dictionary>("projects");

  assert(allProjects != nullptr && globalSourceSites != nullptr && !projectNameOption.empty());

  // if we want to fetch all just call ourselves recursively for every project.
  if(projectNameOption == "all") {
    for(auto currProject = allProjects->begin(); currProject != allProjects->end(); currProject++){
      ((Options&)options).setProject(*currProject);
      ret = FetchAction::Run(user, processContext, processLauncher, filesystem, options);
      if (ret != 0)
        return -1;
    }
  }

  // get project info
  pdbuild::Project project(processContext, options);
  // check for source_site override
  projectSourceSiteOverride = project.getProjectSourceSite();

  // project specific "source_site" overrides global "source_site"
  if (projectSourceSiteOverride != nullptr) {
    auto sourceSite = projectSourceSiteOverride;
    ret = ProcessSourceSite(processContext, filesystem, sourceSite, &project, archivePath);
  } else {
    // loop every source_site
    auto sourceSites = globalSourceSites;
    for (auto sourceSiteObj = sourceSites->begin(); sourceSiteObj != (sourceSites)->end(); sourceSiteObj++) {
      auto sourceSite = plist::CastTo<plist::Dictionary>(sourceSiteObj->get());
      ret = ProcessSourceSite(processContext, filesystem, sourceSite, &project, archivePath);
      // if we downloaded successfully, break.
      if(ret == 0) break;
    }
  }

  if (ret != 0)
    return ret;

  // if we reach here, the project archive is in source cache directory
  // check that the dmg is mounted.
  MountSystemImage(processContext, processLauncher, filesystem, processContext->getDestinationImagePath());
  // create relevant directories
  if(!filesystem->exists(processContext->getDestinationSourcesPath()))
    filesystem->createDirectory(processContext->getDestinationSourcesPath(), false);

  // extract
  ExtractSource(processContext, processLauncher, filesystem, archivePath, project.getProjectNameAndVersion());

  // create receipts for source

  return ret;
}

pdbuild::FetchAction::FetchAction() = default;
pdbuild::FetchAction::~FetchAction() = default;
