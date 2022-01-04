/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <pdbuild/Action.h>
#include <pdbuild/Actions/InstallAction.h>
#include <pdbuild/Options.h>
#include <pdbuild/Usage.h>

#include <libutil/Base.h>
#include <libutil/FSUtil.h>
#include <libutil/Filesystem.h>
#include <process/PDBContext.h>
//#include <xcexecution/Context.h>

#include <unistd.h>
#include <vector>
#include <cerrno>
#include <iostream>
#include <regex>
#include <sys/stat.h>

using pdbuild::InstallAction;
using pdbuild::Options;
using libutil::Filesystem;
using libutil::FSUtil;

int InstallAction::Run(process::User const *user, process::PDBContext const *processContext, process::Launcher *processLauncher, libutil::Filesystem *filesystem, Options const &options)
{

//  if (options.)

  return 0;
}

InstallAction::InstallAction() = default;
InstallAction::~InstallAction() = default;
