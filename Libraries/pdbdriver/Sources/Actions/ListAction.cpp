/**
Copyright (c) 2016-present, Facebook, Inc.
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

#include <libutil/FSUtil.h>
#include <libutil/Filesystem.h>
#include <pdbuild/Actions/ListAction.h>
#include <pdbuild/Usage.h>
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

#include <vector>
#include <iostream>

using libutil::Filesystem;
using pdbuild::Usage;
using libutil::FSUtil;
using libutil::Filesystem;
using pdbuild::ListAction;

int
ListAction::Run(process::User const *user,
                process::PDBContext const *processContext,
                process::Launcher *processLauncher,
                libutil::Filesystem *filesystem, Options const &options)

{
  plist::Dictionary *projects;
  processContext->openPlist(filesystem); // open plist

  projects = processContext->getAllProjects();
  std::cout << "Projects:" << std::endl;

  for (const auto & project : *projects)
    std::cout << "         " << project << std::endl;

  return 0;
}

ListAction::ListAction() = default;
ListAction::~ListAction() = default;