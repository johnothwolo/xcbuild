/**
 Copyright (c) 2016-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#ifndef pdbuild_Usage_h
#define pdbuild_Usage_h

#include <string>

namespace pdbuild {

class Usage {
private:
    Usage();
   ~Usage();

public:
    enum class Option {
        Usage,
        Help,
        InitHelp
    };
    /*
     * Text explaining the usage of the driver.
     * "help" specifies whether to print usage or usage+help.
     */
    static std::string Text(std::string const &name, Option type);
};

}

#endif // !pdbuild_Usage_h
