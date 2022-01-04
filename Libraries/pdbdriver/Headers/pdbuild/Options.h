//
//  Options.hpp
//  xcdarwinbuild
//
//  Created by John Othwolo on 10/20/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//
/**
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 * _
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef pdbuildOptions_hpp
#define pdbuildOptions_hpp


#include <libutil/Options.h>
#include <string>
#include <vector>
#include <utility>

namespace pdbuild {

class Options {

    // init options
private:
    ext::optional<std::string> _init;
    ext::optional<bool>        _noDmg;
    
    // actions
private:
    ext::optional<bool>        _fetch;
    ext::optional<bool>        _install;
    ext::optional<bool>        _list;
    ext::optional<std::string> _group;
    ext::optional<std::string> _globalCodesignIdentity;

    // Additional actions
private:
    ext::optional<bool>        _version;
    ext::optional<bool>        _usage;
    ext::optional<bool>        _help;
    ext::optional<bool>        _license;

    // Options
private:
    ext::optional<bool>        _onlyDeps;
    ext::optional<bool>        _noInstall;
    ext::optional<bool>        _noDeps;
    ext::optional<bool>        _noPatch;
    ext::optional<std::string> _target;
    ext::optional<std::string> _configuration;
    ext::optional<std::string> _osBuild;
    ext::optional<std::string> _project; // project to build
    ext::optional<std::string> _projectVersion; // project version
    ext::optional<std::string> _codesignIdentity;

    // Misc
private:
    ext::optional<bool>        _verbose;
    ext::optional<bool>        _quiet;
    ext::optional<bool>        _json;
    
private:
    std::string                _buildroot; // build system root
    

public:
    Options();
   ~Options();
    
    /* Getter Methods */
public:
    
    // init options
    [[nodiscard]]
    ext::optional<std::string>      const &init() const { return _init; };
    [[nodiscard]]
    bool                            noDmg()     const { return _noDmg.value_or(false); };
    
    // actions
    [[nodiscard]]
    bool                            onlyDeps()   const { return _onlyDeps.value_or(false); };
    [[nodiscard]]
    bool                            noInstall()     const { return _noInstall.value_or(false); };
    [[nodiscard]]
    bool                            fetch()    const { return _fetch.value_or(false); };
    [[nodiscard]]
    bool                            install()      const { return _install.value_or(false); };
    [[nodiscard]]
    ext::optional<std::string>      const &group() const { return _group; };
    [[nodiscard]]
    bool                            list()    const { return _list.value_or(false); };
    [[nodiscard]]
    ext::optional<std::string>      const &globalCodesignIdentity() const { return _globalCodesignIdentity; };

    // Options
    [[nodiscard]]
    bool                            noPatch()   const { return _noPatch.value_or(false); };
    [[nodiscard]]
    bool                            noDeps()    const { return _noDeps.value_or(false); };
    [[nodiscard]]
    ext::optional<std::string>      const &target()           const { return _target; };
    [[nodiscard]]
    ext::optional<std::string>      const &configuration()    const { return _configuration; };
    [[nodiscard]]
    ext::optional<std::string>      const &osBuild()          const { return _osBuild; };
    [[nodiscard]]
    ext::optional<std::string>      const &codesignIdentity() const { return _codesignIdentity; };
    [[nodiscard]]
    ext::optional<std::string>      const &project()          const { return _project; };
    [[nodiscard]]
    ext::optional<std::string>      const &projectVersion()   const { return _projectVersion; };
    
    // Extension: Additional actions
    [[nodiscard]]
    bool                            usage()     const { return _usage.value_or(false); };
    [[nodiscard]]
    bool                            help()      const { return _help.value_or(false); };
    [[nodiscard]]
    bool                            license()   const { return _license.value_or(false); };
    [[nodiscard]]
    bool                            version()   const { return _version.value_or(false); };
    
    // Extension: Misc
    [[nodiscard]]
    bool                            verbose()   const { return _verbose.value_or(false); };
    [[nodiscard]]
    bool                            quiet()     const { return _quiet.value_or(false); };
    [[nodiscard]]
    bool                            json()      const { return _json.value_or(false); };
    
    // buildroot
    void                            setBuildRoot(std::string br) { _buildroot = br; };
    [[nodiscard]]
    std::string              const &buildroot()     const { return _buildroot; };

public:
    void setProject(const std::string &project) { _project = project; };

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

} // namespace pdbuild

#endif /* pdbuildOptions_hpp */
