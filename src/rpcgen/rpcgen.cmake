# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang         <@liangbin>
#
# Summary: rpcgen CMake listfile for zeta project

message("Processing src/rpcgen/CMakeList.txt")

set(RPCGEN_SRC trn_rpc_protocol.x)

set(RPCGEN_FILES
  "${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol.h"
  "${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol_xdr.c"
  "${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol_svc.c"
  "${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol_clnt.c"
)

set(RPCGEN_SVC
  "${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol_xdr.c"
  "${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol_svc.c"
)

set(RPCGEN_CLNT
  "${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol_xdr.c"
  "${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol_clnt.c"
)

add_custom_command(
  OUTPUT ${RPCGEN_FILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/rpcgen
  COMMAND rpcgen -h -L -C -o ${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol.h ${RPCGEN_SRC}
  COMMAND rpcgen -c -L -C -o ${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol_xdr.c ${RPCGEN_SRC}
  COMMAND rpcgen -m -L -C -o ${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol_svc.c ${RPCGEN_SRC}
  COMMAND rpcgen -l -L -C -o ${CMAKE_CURRENT_BINARY_DIR}/rpcgen/trn_rpc_protocol_clnt.c ${RPCGEN_SRC}
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/${RPCGEN_SRC}
  COMMENT "generating RPC API source code"
  VERBATIM
)

add_custom_target(
    rpcgen
    DEPENDS ${RPCGEN_FILES}
)

# ----------------------------------------------------------------------------
# Append header file location to INCLUDE_DIRECTORIES 
include_directories("${CMAKE_CURRENT_BINARY_DIR}/rpcgen")
