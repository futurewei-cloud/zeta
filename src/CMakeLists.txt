# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang         <@liangbin>
#
# Summary: src level CMake listfile for zeta project

message("Processing src/CMakeList.txt")

include(extern/extern.cmake)
include(rpcgen/rpcgen.cmake)
include(cli/CMakeLists.txt)
include(dmn/CMakeLists.txt)

add_subdirectory(mgmt)
add_subdirectory(xdp)
