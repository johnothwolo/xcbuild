/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <libutil/FSUtil.h>
#include <pdbuild/Actions/VersionAction.h>
#include <process/PDBContext.h>

#include <cstdio>

using pdbuild::VersionAction;
using libutil::FSUtil;
using libutil::Filesystem;

VersionAction::
VersionAction(){}

VersionAction::
~VersionAction(){}

int VersionAction::
Run(process::User const *user, process::PDBContext const *processContext, Filesystem const *filesystem, Options const &options)
{
    std::string path = processContext->executablePath();
    // TODO: Implement version number.
    printf("pdbuild version 0.1\n");
    printf("Build version 1\n");
    return 0;
}
