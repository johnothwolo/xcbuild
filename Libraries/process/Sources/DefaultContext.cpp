/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree.
 */

#include <process/DefaultContext.h>
#include <libutil/FSUtil.h>

#include <mutex>
#include <sstream>
#include <unordered_set>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#ifdef __OpenBSD__
#include <glob.h>
#else
#include <wordexp.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <linux/limits.h>
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 16
#include <sys/auxv.h>
#endif
#elif defined(__FreeBSD__)
#define _LIMITS_H_
#include <sys/syslimits.h>
#elif defined(__OpenBSD__)
#include <sys/sysctl.h>
#endif

extern "C" char **environ;

using process::DefaultContext;
using libutil::FSUtil;

DefaultContext::
DefaultContext() :
    Context()
{
}

DefaultContext::
~DefaultContext()
{
}

std::string const &DefaultContext::
currentDirectory() const
{
    static std::string const *directory = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
        std::string path;
        for (size_t size = PATH_MAX; true; size *= 2) {
            std::string current = std::string();
            current.resize(size);

            char *ret = ::getcwd(&current[0], current.size());
            if (ret != nullptr) {
                /* Success. */
                path = std::string(current.c_str());
                break;
            } else if (errno == ERANGE) {
                /* Needs more space. */
            } else {
                abort();
            }
        }
        directory = new std::string(path);
    });

    return *directory;
}

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static char initialWorkingDirectory[PATH_MAX] = { 0 };
__attribute__((constructor))
static void InitializeInitialWorkingDirectory()
{
    if (getcwd(initialWorkingDirectory, sizeof(initialWorkingDirectory)) == NULL) {
        abort();
    }
}

#if (!(__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 16) || defined(__FreeBSD__)) && !defined(__OpenBSD__)
static char initialExecutablePath[PATH_MAX] = { 0 };
__attribute__((constructor))
static void InitialExecutablePathInitialize(int argc, char **argv)
{
    strncpy(initialExecutablePath, argv[0], sizeof(initialExecutablePath));
}
#elif defined(__OpenBSD__)
static char initialExecutablePath[PATH_MAX] = { 0 };
__attribute__((constructor))
static void InitialExecutablePathInitialize()
{
    int mib[4];
    char **argv;
    size_t len;
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC_ARGS;
    mib[2] = getpid();
    mib[3] = KERN_PROC_ARGV;
    if (sysctl(mib, 4, NULL, &len, NULL, 0) < 0)
        abort();
    if (!(argv = (char**) malloc(len)))
        abort();
    if (sysctl(mib, 4, argv, &len, NULL, 0) < 0)
        abort();
    strncpy(initialExecutablePath, argv[0], sizeof(initialExecutablePath));
    free(argv);
}
#endif
#endif

std::string const &DefaultContext::
executablePath() const
{
    static std::string const *executablePath = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
        std::string absolutePath;

#if defined(__APPLE__)
        uint32_t size = 0;
        if (_NSGetExecutablePath(NULL, &size) != -1) {
            abort();
        }

        absolutePath.resize(size - 1); /* Size includes terminator. */
        if (_NSGetExecutablePath(&absolutePath[0], &size) != 0) {
            abort();
        }
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#if !defined(__FreeBSD__) && !defined(__OpenBSD__) && __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 16
        char const *path = reinterpret_cast<char const *>(getauxval(AT_EXECFN));
        if (path == NULL) {
            abort();
        }
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__GLIBC__)
        char const *path = reinterpret_cast<char const *>(initialExecutablePath);
#else
#error Requires glibc on Linux.
#endif
        absolutePath = FSUtil::ResolveRelativePath(std::string(path), std::string(initialWorkingDirectory));
#else
#error Unsupported platform.
#endif

        executablePath = new std::string(FSUtil::NormalizePath(absolutePath));
    });

    return *executablePath;
}

#if defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
static int commandLineArgumentCount = 0;
static char **commandLineArgumentValues = NULL;

#if defined(__APPLE__)
__attribute__((constructor))
#endif
static void CommandLineArgumentsInitialize(int argc, char **argv)
{
    commandLineArgumentCount = argc;
    commandLineArgumentValues = argv;
}

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
__attribute__((section(".init_array"))) auto commandLineArgumentInitializer = &CommandLineArgumentsInitialize;
#endif
#endif

std::vector<std::string> const &DefaultContext::
commandLineArguments() const
{
    static std::vector<std::string> const *arguments = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
#if defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        arguments = new std::vector<std::string>(commandLineArgumentValues + 1, commandLineArgumentValues + commandLineArgumentCount);
#else
#error Unsupported platform.
#endif
    });

    return *arguments;
}

ext::optional<std::string> DefaultContext::
environmentVariable(std::string const &variable) const
{
    if (char *value = ::getenv(variable.c_str())) {
        return std::string(value);
    } else {
        return ext::nullopt;
    }
}

std::unordered_map<std::string, std::string> const &DefaultContext::
environmentVariables() const
{
    static std::unordered_map<std::string, std::string> const *environment = nullptr;

    static std::once_flag flag;
    std::call_once(flag, []{
        std::unordered_map<std::string, std::string> values;
        for (char **current = environ; *current; current++) {
            std::string variable = *current;

            std::string::size_type offset = variable.find('=');
            std::string name = variable.substr(0, offset);
            std::string value = variable.substr(offset + 1);
            values.insert({ name, value });
        }
        environment = new std::unordered_map<std::string, std::string>(values);
    });

    return *environment;
}

ext::optional<std::string> const DefaultContext::
shellExpand(std::string const &s) const
{
    std::string expandedString = s;
#if __OpenBSD__
    glob_t result;
    if (glob(s.c_str(), 0, NULL, &result) == 0) {
        if (result.gl_pathc != 1) {
           abort();
	}
	expandedString = std::string(result.gl_pathv[0]);
	globfree(&result);
    }
#else
    wordexp_t result;
    if (wordexp(s.c_str(), &result, 0) == 0) {
        if (result.we_wordc != 1) {
           abort();
        }
        expandedString = std::string(result.we_wordv[0]);
        wordfree(&result);
    }
#endif
    return expandedString;
}
