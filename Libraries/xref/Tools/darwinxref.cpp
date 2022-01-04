/**
 * Copyright (c) 2021-present, PureDarwin.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <Xref.h>
#include <Plugin.h>
#include <libutil/DefaultFilesystem.h>
#include <libutil/FSUtil.h>
#include <libutil/Options.h>
#include <process/DefaultContext.h>

#include <any>
#include <fstream>
#include <iostream>
#include <iterator>
#include <libgen.h>

using libutil::DefaultFilesystem;
using libutil::FSUtil;
using plugin::Plugin;

class Options {
public:
  Options();
  ~Options();

private:
  ext::optional<bool>         _help;
  ext::optional<std::string>  _plugin;
  std::vector<std::string>    _pluginArgs;

public:
  [[nodiscard]]
  bool help() const { return _help.value_or(false); }
  [[nodiscard]]
  std::vector<std::string> const &pluginArgs() const { return _pluginArgs; }
  [[nodiscard]]
  ext::optional<std::string> const &plugin() const { return _plugin; }

private:
  friend class libutil::Options;
  std::pair<bool, std::string>
  parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

Options::Options() = default;
Options::~Options() = default;

std::pair<bool, std::string> Options::
parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it)
{
  std::string const &arg = **it;

  if (arg == "-usage" || arg == "-help") {
    return libutil::Options::Current<bool>(&_help, arg);
  } else if (!arg.empty() && arg[0] != '-') {
    _pluginArgs.push_back(arg);
    return std::make_pair(true, std::string());
  } else {
    return std::make_pair(false, "unknown argument " + arg);
  }
}

static int
Help(const process::DefaultContext& processContext,
     Plugin<std::any> *plugin = nullptr) {
  if (plugin != nullptr) {
    std::cerr << plugin->getName() << ": invalid or insufficient arguments\n";
    std::cerr << "usage: "
              << ::basename((char *)processContext.executablePath().c_str())
              << " " << plugin->getName() << " " << plugin->usage() << "\n";
    return 0;
  } else {
    std::cerr << "unknown command \"" << processContext.commandLineArguments().front() << "\" \n";
    std::cerr << "usage: "
              << ::basename((char *)processContext.executablePath().c_str())
              << " [-f db] [-b build] <command> ...\n";
    std::cerr << "commands:\n";
    plugin::PrintPluginList();
  }
  return 0;
}

static std::string getBuild(DefaultFilesystem *filesystem, const std::string& buildroot){
  auto path = buildroot+"/.pdbuild/build";
  std::string buildString;
  if (!filesystem->exists(path))
    throw "Build file Not found";
  std::fstream file(path);
  file >> buildString;
  file.close();
  return buildString;
}
// DARWINXREF_DB_FILE env flag...
int
main(int argc, char **argv)
{
  Options options;
  std::pair<bool, std::string> result;
  std::string buildRoot, dbpath, build;
  [[maybe_unused]] DefaultFilesystem filesystem = DefaultFilesystem();
  process::DefaultContext processContext = process::DefaultContext();
  std::vector<std::string> args = processContext.commandLineArguments();
  auto plugin = (Plugin<std::any> *) plugin::GetPluginWithName(args.front());
  int ret = 0;

  if (plugin == nullptr)
    return Help(processContext);

  // erase plugin argument.
  args.erase(args.begin());

  // get cwd for buildroot (or override with env var)
  buildRoot = processContext.environmentVariable("PDB_BUILDROOT").value_or(processContext.currentDirectory());
  // get db path
  dbpath = buildRoot + "/.pdbuild/xref.db";

  // get build number
  build = processContext.environmentVariable("PDB_BUILD").value_or(getBuild(&filesystem, buildRoot));

  // check commandline args
  result = libutil::Options::Parse<Options>(&options, processContext.commandLineArguments());
  if (!result.first) {
    return Help(processContext, nullptr);
  }

  // verify db path
  if(!filesystem.exists(dbpath)) {
    std::cerr << "xref.db not found";
    return EBADF;
  }
  // run plugin
  auto db = plugin::DBDatabase(dbpath);
  ret = plugin->run(db, build, args);

  if (ret == EINVAL)
    return Help(processContext, plugin);
  plugin = NULL;
  return ret;
}
