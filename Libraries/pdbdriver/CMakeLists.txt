#
# Copyright (c) 2021-present, PureDarwin.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.
#

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -fexceptions)

add_library(pdbdriver STATIC
        Sources/Options.cpp
        Sources/Action.cpp
        Sources/Driver.cpp
        Sources/ProgramExecutor.cpp
        Sources/Receipt.cpp
        Sources/Usage.cpp
        #
        Sources/Actions/BuildAction.cpp
        Sources/Actions/FetchAction.cpp
        Sources/Actions/GroupAction.cpp
        Sources/Actions/InstallAction.cpp
        Sources/Actions/HelpAction.cpp
        Sources/Actions/InitAction.cpp
        Sources/Actions/LicenseAction.cpp
        Sources/Actions/CodesignAction.cpp
        Sources/Actions/UsageAction.cpp
        Sources/Actions/VersionAction.cpp
        Sources/Actions/ListAction.cpp Sources/Project.cpp)

find_library(CORE_FOUNDATION CoreFoundation)
if ("${CORE_FOUNDATION}" STREQUAL "CORE_FOUNDATION-NOTFOUND")
    set(CORE_FOUNDATION "")
endif ()

find_library(CORE_SERVICES CoreServices)
if ("${CORE_SERVICES}" STREQUAL "CORE_SERVICES-NOTFOUND")
    set(CORE_SERVICES "")
endif ()

target_link_libraries(pdbdriver PUBLIC util xref plist pbxsetting ${CORE_FOUNDATION} ${CORE_SERVICES})
target_include_directories(pdbdriver PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
install(TARGETS pdbdriver DESTINATION usr/lib)

add_executable(pdbuild Tools/pdbuild.cpp)
target_link_libraries(pdbuild pdbdriver process util)
install(TARGETS pdbuild DESTINATION usr/bin)

#if (BUILD_TESTING)
#    ADD_UNIT_GTEST(builtin copy Tests/test_copy.cpp)
#    ADD_UNIT_GTEST(builtin copyStrings Tests/test_copyStrings.cpp)
#    ADD_UNIT_GTEST(builtin copyPlist Tests/test_copyPlist.cpp)
#endif ()
