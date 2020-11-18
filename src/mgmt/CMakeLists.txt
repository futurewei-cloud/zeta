# SPDX-License-Identifier: MIT
#
# Copyright (c) 2020 The Authors.
# Authors: Bin Liang         <@liangbin>
#
# Summary: mgmt CMake listfile for zeta project

message("Processing src/mgmt/CMakeList.txt")

set(PROTO_READY ${CMAKE_BINARY_DIR}/src/mgmt/operator/proto.ready)

file(GLOB PROTO_PATH_FILES ${CMAKE_CURRENT_LIST_DIR}/operator/proto/def/*.proto)
string(REPLACE "${CMAKE_CURRENT_LIST_DIR}/operator/proto/def/" "" PROTO_FILES "${PROTO_PATH_FILES}")

set(PROTO_FLAGS 
    -I${CMAKE_CURRENT_LIST_DIR}/operator/proto/def
    --grpc_python_out=${CMAKE_BINARY_DIR}/src/mgmt/operator/proto
    --python_out=${CMAKE_BINARY_DIR}/src/mgmt/operator/proto
)

add_custom_command(
  OUTPUT ${PROTO_READY}
  COMMAND mkdir -p ${CMAKE_BINARY_DIR}/src/mgmt/operator/proto
  COMMAND python3 -m grpc_tools.protoc ${PROTO_FLAGS} ${PROTO_FILES}
  COMMAND cmake -E touch ${PROTO_READY}
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/operator/proto/def/*.proto
  COMMENT "generating Protobuf and gRPC APIs"
  VERBATIM
)

add_custom_target(
    proto ALL
    DEPENDS ${PROTO_READY}
)
