/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <process/DefaultUser.h>

#include <mutex>
#include <sstream>
#include <string>
#include <cstring>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

using process::DefaultUser;

DefaultUser::
DefaultUser() :
    User()
{
}

DefaultUser::
~DefaultUser()
{
}

std::string const &DefaultUser::
userID() const
{
    static std::string const *userID = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
        std::ostringstream os;
        os << ::getuid();
        userID = new std::string(os.str());
    });

    return *userID;
}

std::string const &DefaultUser::
groupID() const
{
    static std::string const *groupID = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
        std::ostringstream os;
        os << ::getgid();
        groupID = new std::string(os.str());
    });

    return *groupID;
}

std::string const &DefaultUser::
userName() const
{
    static std::string const *userName = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
        if (struct passwd const *pw = ::getpwuid(::getuid())) {
            if (pw->pw_name != nullptr) {
                userName = new std::string(pw->pw_name);
            }
        }

        if (userName == nullptr) {
            std::ostringstream os;
            os << ::getuid();
            userName = new std::string(os.str());
        }
    });

    return *userName;
}

std::string const &DefaultUser::
groupName() const
{
    static std::string const *groupName = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
        if (struct group const *gr = ::getgrgid(::getgid())) {
            if (gr->gr_name != nullptr) {
                groupName = new std::string(gr->gr_name);
            }
        }

        if (groupName == nullptr) {
            std::ostringstream os;
            os << ::getgid();
            groupName = new std::string(os.str());
        }
    });

    return *groupName;
}

ext::optional<std::string> DefaultUser::
userHomeDirectory() const
{
    char *home = ::getpwuid(::getuid())->pw_dir;
    if (home != nullptr) {
        return std::string(home);
    } else {
        return ext::nullopt;
    }
}
