# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang         <@liangbin>
#
# Summary: xdp CMake listfile for zeta project

message("Processing src/xdp/CMakeList.txt")

set(LLC llc-7)
set(LLC_FLAGS -march=bpf -filetype=obj)
set(CLANG clang-7)
set(CLANG_FLAGS -I.
    -I${CMAKE_SOURCE_DIR}/src
    -I${CMAKE_SOURCE_DIR}/src/include
    -Wno-unused-value
    -Wno-pointer-sign
    -Wno-compare-distinct-pointer-types
    -Wno-gnu-variable-sized-type-not-at-end
    -Wno-address-of-packed-member
    -Wno-tautological-compare
    -Wno-unknown-warning-option
    -O3
    -emit-llvm
    -c)
set(CLANG_FLAGS_DEBUG -DDEBUG -D__KERNEL__ -g -D__BPF_TRACING__ ${CLANG_FLAGS})

set(XDP_READY ${CMAKE_BINARY_DIR}/xdp.ready)
set(OBJDIR ${CMAKE_BINARY_DIR}/src/xdp)
file(GLOB XDP_PATH_FILES ${CMAKE_CURRENT_LIST_DIR}/*.c)
string(REPLACE "${CMAKE_CURRENT_LIST_DIR}/" "" XDP_FILES "${XDP_PATH_FILES}")

add_custom_command(
  OUTPUT ${XDP_READY}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  COMMAND mkdir -p ${OBJDIR}
  COMMAND mkdir -p ${CMAKE_BINARY_DIR}/xdp
  COMMAND for file in `find . -name \"*.c\"`\; do fname=\$\$\(basename -- \"\$\$\{file%.*\}\"\) && ${CLANG} \$\$\{fname\}.c ${CLANG_FLAGS} -o ${OBJDIR}/\$\$\{fname\}_ebpf.bc && ${CLANG} \$\$\{fname\}.c ${CLANG_FLAGS_DEBUG} -o ${OBJDIR}/\$\$\{fname\}_ebpf_debug.bc \; done
  COMMAND cd ${OBJDIR} && for file in `find . -name \"*.bc\"`\; do ${LLC} ${LLC_FLAGS} \$\$file \; done
  COMMAND cp ${OBJDIR}/*.o ${CMAKE_BINARY_DIR}/xdp
  COMMAND cmake -E touch ${XDP_READY}
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/*.c ${CMAKE_CURRENT_LIST_DIR}/*.h
  COMMENT "generating eBPF objects"
)

add_custom_target(
    xdp ALL
    DEPENDS ${XDP_READY}
)
