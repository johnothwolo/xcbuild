/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <pdbuild/Actions/LicenseAction.h>

#include <string>
#include <iostream>
#include <cstdio>

/*
 * Generated from project license. The check below is extra-conservative
 * in that it fails if __has_include is not available (even though the
 * header may be), since the mechanism is more complex than a usual include.
 */

static char const __BSD_LICENSE__[] =
"                                                                                       \n \
Copyright © 2021 The PureDarwin Project.                                                \n \
All rights reserved.                                                                    \n \
                                                                                        \n \
Redistribution and use in source and binary forms, with or without modification,        \n \
are permitted provided that the following conditions are met:                           \n \
                                                                                        \n \
 1. Redistributions of source code must retain the above copyright                      \n \
    notice, this list of conditions and the following disclaimer.                       \n \
                                                                                        \n \
 2. Redistributions in binary form must reproduce the above copyright notice,           \n \
    this list of conditions and the following disclaimer in the documentation           \n \
    and/or other materials provided with the distribution.                              \n \
                                                                                        \n \
 3. Neither the name of the organization nor the names of its contributors may          \n \
    be used to endorse or promote products derived from this software without           \n \
    specific prior written permission.                                                  \n \
                                                                                        \n \
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY   \n \
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED               \n \
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.      \n \
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY                \n \
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES              \n \
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF    \n \
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    \n \
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING                    \n \
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,            \n \
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                      \n \
";

#if defined(__has_include)
#if __has_include("LICENSE.h")
#include "LICENSE.h"
#else
static std::string const LICENSE = __BSD_LICENSE__;
#endif
#else
static std::string const LICENSE = __BSD_LICENSE__;
#endif

using pdbuild::LicenseAction;

int LicenseAction::Run(){
    std::cout << LICENSE << std::endl;
    return 0;
}

LicenseAction::LicenseAction() = default;
LicenseAction::~LicenseAction() = default;
