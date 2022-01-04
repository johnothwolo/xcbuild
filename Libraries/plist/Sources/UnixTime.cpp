/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <plist/UnixTime.h>

#include <ctime>

using plist::UnixTime;

void UnixTime::
Decode(uint64_t in, struct tm &out)
{
    time_t t = in;
    ::gmtime_r(&t, &out);
}

uint64_t UnixTime::
Encode(struct tm const &in)
{
    struct tm copy = in;
    return ::mktime(&copy);
}
