//
//  Options.cpp
//  xcdarwinbuild
//
//  Created by John Othwolo on 10/20/21.
//  Copyright © 2021 Pure Darwin. All rights reserved.
//
/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <pdbuild/Options.h>

using pdbuild::Options;

Options:: Options(){}
Options::~Options(){}

std::pair<bool, std::string> Options::
parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it)
{
    std::string const &arg = **it;
    
    if (arg == "-usage") {
        return libutil::Options::Current<bool>(&_usage, arg);
    } else if (arg == "-help") {
        return libutil::Options::Current<bool>(&_help, arg);
    } else if (arg == "-license") {
        return libutil::Options::Current<bool>(&_license, arg);
    } else if (arg == "--version") {
        return libutil::Options::Current<bool>(&_version, arg);
//    } else if (arg == "-verbose") {
//        return libutil::Options::Current<bool>(&_verbose, arg);
//    } else if (arg == "-quiet") {
//        return libutil::Options::Current<bool>(&_quiet, arg);
//    } else if (arg == "-json") {
//        return libutil::Options::Current<bool>(&_json, arg);
    }
    // Init
    else if (arg == "-init") {
        return libutil::Options::Next<std::string>(&_init, args, it);
    } else if (arg == "-nodmg") {
        return libutil::Options::Current<bool>(&_noDmg, arg);
    }
    // Actions
    else if (arg == "-fetch") {
      return libutil::Options::Current<bool>(&_fetch, arg);;
    } else if (arg == "-install") {
      return libutil::Options::Current<bool>(&_install, arg);;
    } else if (arg == "-group") {
      return libutil::Options::Next<std::string>(&_group, args, it);
    } else if (arg == "-list") {
      return libutil::Options::Current<bool>(&_list, arg);
    } else if (arg == "-setCodesign") {
      return libutil::Options::Next<std::string>(&_globalCodesignIdentity, args, it);
    }
    // Options
    else if (arg == "-onlyDeps") {
      return libutil::Options::Current<bool>(&_onlyDeps, arg);
    } else if (arg == "-noInstall") {
      return libutil::Options::Current<bool>(&_noInstall, arg);
    } else if (arg == "-noDeps") {
      return libutil::Options::Current<bool>(&_noDeps, arg);
    } else if (arg == "-noPatch") {
      return libutil::Options::Current<bool>(&_noPatch, arg);
    } else if (arg == "-target") {
      return libutil::Options::Next<std::string>(&_target, args, it);
    } else if (arg == "-configuration") {
        return libutil::Options::Next<std::string>(&_configuration, args, it);
    } else if (arg == "-build") {
        return libutil::Options::Next<std::string>(&_osBuild, args, it);
    } else if (arg == "-version") {
        return libutil::Options::Next<std::string>(&_projectVersion, args, it);
    } else if (arg == "-codesign") {
        return libutil::Options::Next<std::string>(&_codesignIdentity, args, it);
    } else if (!arg.empty() && arg[0] != '-') {
        if (_project->empty())
             return libutil::Options::Current<std::string>(&_project, arg);
//        else if (!_projectVersion->empty())
//             return libutil::Options::Next<std::string>(&_projectVersion, args, it);
        else return std::make_pair(false, "excess argument " + arg);
        
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

