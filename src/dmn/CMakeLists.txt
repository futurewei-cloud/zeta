# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang         <@liangbin>
#
# Summary: dmn CMake listfile for zeta project

message("Processing src/dmn/CMakeList.txt")

file(GLOB SOURCE ${CMAKE_CURRENT_LIST_DIR}/*.c)

add_executable(transitd ${RPCGEN_SVC} ${SOURCE})
add_dependencies(transitd libbpf rpcgen)
target_link_libraries(transitd -l:libbpf.a -l:libelf.a -lz -lnsl -pthread -lrt)
set_target_properties(transitd PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)

file(GLOB TEST_SOURCE ${CMAKE_CURRENT_LIST_DIR}/test/*.c ${CMAKE_CURRENT_LIST_DIR}/*.c)
list(REMOVE_ITEM TEST_SOURCE ${CMAKE_CURRENT_LIST_DIR}/trn_transitd.c)
set(DMN_MOCKS "-Wl,--wrap=bpf_map_update_elem \
    -Wl,--wrap=bpf_map_lookup_elem \
    -Wl,--wrap=bpf_map_delete_elem \
    -Wl,--wrap=bpf_map_lookup_and_delete_elem \
    -Wl,--wrap=bpf_prog_load_xattr \
    -Wl,--wrap=bpf_set_link_xdp_fd \
    -Wl,--wrap=bpf_obj_get_info_by_fd \
    -Wl,--wrap=bpf_map__next \
    -Wl,--wrap=bpf_map__fd \
    -Wl,--wrap=bpf_map__pin \
    -Wl,--wrap=bpf_map__unpin \
    -Wl,--wrap=bpf_get_link_xdp_id \
    -Wl,--wrap=bpf_object__open \
    -Wl,--wrap=bpf_create_map \
    -Wl,--wrap=bpf_program__fd \
    -Wl,--wrap=bpf_object__load \
    -Wl,--wrap=bpf_object__find_map_by_name \
    -Wl,--wrap=bpf_map__set_inner_map_fd \
    -Wl,--wrap=bpf_program__set_xdp \
    -Wl,--wrap=bpf_program__next \
    -Wl,--wrap=bpf_object__close \
    -Wl,--wrap=if_nametoindex \
    -Wl,--wrap=if_indextoname \
    -Wl,--wrap=trn_transit_map_get_fd \
    -Wl,--wrap=setrlimit \
    -Wl,--wrap=close \
    -Wl,--wrap=bpf_program__set_ifindex \
    -Wl,--wrap=bpf_program__set_type \
    -Wl,--wrap=bpf_object__name \
    -Wl,--wrap=bpf_map__name \
    -Wl,--wrap=bpf_map__set_ifindex \
    -Wl,--wrap=bpf_obj_get \
    -Wl,--wrap=bpf_map__reuse_fd \
    -Wl,--wrap=bpf_object__open_xattr")

add_executable(test_dmn ${RPCGEN_SVC} ${TEST_SOURCE})
add_dependencies(test_dmn libbpf rpcgen)
# Add test coverage compiler flags
target_compile_options(test_dmn PRIVATE -fprofile-arcs -ftest-coverage)
# Add CMocka unit test and cJSON to linker flags
target_link_libraries(test_dmn extern -lgcov --coverage -lcmocka ${DMN_MOCKS})
set_target_properties(test_dmn PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/tests
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/tests
)
add_test(NAME test_dmn COMMAND test_dmn)
