# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang         <@liangbin>
#
# Summary: test level CMake listfile for zeta project

message("Processing test/CMakeList.txt")

add_test(NAME test_test COMMAND bash -c "cd ${CMAKE_SOURCE_DIR} && python3 -W ignore -m unittest discover test/ -v")
#add_test(NAME test_mgmt COMMAND bash -c "cd ${CMAKE_SOURCE_DIR} && python3 -W ignore -m unittest zeta.tests.${TEST} -v")
