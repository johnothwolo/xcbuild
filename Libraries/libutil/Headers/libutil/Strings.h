/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#ifndef __libutil_Strings_h
#define __libutil_Strings_h

#include <libutil/Filesystem.h>
#include <strings.h>

namespace libutil {

static inline int strcasecmp(const char *s1, const char *s2) {
  return ::strcasecmp(s1, s2);
}

}

#endif  // !__libutil_Strings_h
