/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <xcdriver/LicenseAction.h>

#include <string>

#include <cstdio>

/*
 * Generated from project license. The check below is extra-conservative
 * in that it fails if __has_include is not available (even though the
 * header may be), since the mechanism is more complex than a usual include.
 */
#if defined(__has_include)
#if __has_include("LICENSE.h")
#include "LICENSE.h"
#else
static char const LICENSE[] = "<unavailable>";
#endif
#else
static char const LICENSE[] = "<unavailable>";
#endif

using xcdriver::LicenseAction;

int LicenseAction::
Run()
{
    std::string license = std::string(LICENSE, sizeof(LICENSE));
    fprintf(stdout, "%s\n", license.c_str());

    return 0;
}

LicenseAction::LicenseAction() = default;
LicenseAction::~LicenseAction() = default;
