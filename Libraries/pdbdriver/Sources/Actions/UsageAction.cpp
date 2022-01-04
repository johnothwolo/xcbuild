/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <libutil/FSUtil.h>
#include <pdbuild/Actions/UsageAction.h>
#include <pdbuild/Usage.h>
#include <process/PDBContext.h>

#include <cstdio>

using pdbuild::UsageAction;
using pdbuild::Usage;
using libutil::FSUtil;

UsageAction::
UsageAction(){}

UsageAction::
~UsageAction(){}

int UsageAction::
Run(process::PDBContext const *processContext)
{
    std::string path = processContext->executablePath();
    std::string text = Usage::Text(FSUtil::GetBaseName(path), Usage::Option::Usage);
    fprintf(stdout, "%s", text.c_str());

    return 0;
}
