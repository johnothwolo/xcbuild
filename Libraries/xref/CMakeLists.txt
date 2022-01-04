#
# Copyright (c) 2021-present, PureDarwin.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.
#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS -fexceptions)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(xref
        Sources/DBDatabase.cpp
        Sources/PluginBase.cpp
        #
        Sources/plugins/binary_sites.cpp
        Sources/plugins/configuration.cpp
        Sources/plugins/currentBuild.cpp
        Sources/plugins/darwin.cpp
        Sources/plugins/dependencies.cpp
        Sources/plugins/descrip.txt
        Sources/plugins/diff.cpp
        Sources/plugins/dot.cpp
        Sources/plugins/edit.cpp
        Sources/plugins/environment.cpp
        Sources/plugins/exportFiles.cpp
        Sources/plugins/exportIndex.cpp
        Sources/plugins/exportProject.cpp
        Sources/plugins/findFile.cpp
        Sources/plugins/group.cpp
        Sources/plugins/inherits.cpp
        Sources/plugins/loadDeps.cpp
        Sources/plugins/loadFiles.cpp
        Sources/plugins/loadIndex.cpp
        Sources/plugins/macosx.cpp
        Sources/plugins/mergeBuild.cpp
        Sources/plugins/original.cpp
        Sources/plugins/patch_sites.cpp
        Sources/plugins/patchfiles.cpp
        Sources/plugins/plist_sites.cpp
        Sources/plugins/query.cpp
        Sources/plugins/registerFiles.cpp
        Sources/plugins/resolveDeps.cpp
        Sources/plugins/source_sites.cpp
        Sources/plugins/target.cpp
        Sources/plugins/version.cpp
        )

#find_library(CORE_FOUNDATION CoreFoundation)
#if ("${CORE_FOUNDATION}" STREQUAL "CORE_FOUNDATION-NOTFOUND")
#    set(CORE_FOUNDATION "")
#endif ()
#
#find_library(CORE_SERVICES CoreServices)
#if ("${CORE_SERVICES}" STREQUAL "CORE_SERVICES-NOTFOUND")
#    set(CORE_SERVICES "")
#endif ()
#

target_include_directories(xref PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
target_link_libraries(xref sqlitemodern sqlite3)

install(TARGETS xref DESTINATION usr/lib)

#target_link_libraries(builtin PUBLIC dependency util plist pbxsetting ${CORE_FOUNDATION} ${CORE_SERVICES})
#target_include_directories(builtin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
#install(TARGETS builtin DESTINATION usr/lib)

add_executable(darwinxref Tools/darwinxref.cpp)
target_link_libraries(darwinxref xref sqlitemodern process util)
install(TARGETS darwinxref DESTINATION usr/bin)


#if (BUILD_TESTING)
#    ADD_UNIT_GTEST(builtin copy Tests/test_copy.cpp)
#    ADD_UNIT_GTEST(builtin copyStrings Tests/test_copyStrings.cpp)
#    ADD_UNIT_GTEST(builtin copyPlist Tests/test_copyPlist.cpp)
#endif ()
