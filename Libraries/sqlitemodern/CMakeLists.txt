#
# Copyright (c) 2021-present, PureDarwin.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.
#

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -fexceptions)

add_library(sqlitemodern Sources/dummy.cpp)

target_include_directories(sqlitemodern PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Headers")
target_include_directories(sqlitemodern PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/PrivateHeaders")
install(TARGETS builtin DESTINATION usr/lib)
