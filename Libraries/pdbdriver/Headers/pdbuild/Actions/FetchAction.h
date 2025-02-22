/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#ifndef pdbuild_FetchAction_h
#define pdbuild_FetchAction_h

#include <string>

namespace libutil { class Filesystem; }
namespace process { class PDBContext; }
namespace process { class Launcher; }
namespace process { class User; }

namespace pdbuild {

class Options;

class FetchAction {
public:
  class SourceHost {

  private:
    SourceHost() = default;
    ~SourceHost() = default;
  public:
    static const std::string Github;
    static const std::string ArchiveUrl;
    static const std::string Apple;
    static const std::string Local;
  };

private:
    FetchAction();
    ~FetchAction();

public:
  static int
  Run(process::User const *user, process::PDBContext const *processContext, process::Launcher *processLauncher, libutil::Filesystem *filesystem, Options const &options);
};

}

#endif // !pdbuild_FetchAction_h
