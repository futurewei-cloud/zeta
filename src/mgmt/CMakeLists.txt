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

set(OPERATOR_STAMP ${CMAKE_BINARY_DIR}/operator.stamp)

add_custom_command(
  OUTPUT ${OPERATOR_STAMP}
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/operator
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/operator ${CMAKE_BINARY_DIR}/operator
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/common ${CMAKE_BINARY_DIR}/operator/common
  COMMAND ${CMAKE_COMMAND} -E touch ${OPERATOR_STAMP}
  VERBATIM
)

add_custom_target(
    operator ALL
    DEPENDS ${OPERATOR_STAMP}
)

set(MANAGER_STAMP ${CMAKE_BINARY_DIR}/manager.stamp)

add_custom_command(
  OUTPUT ${MANAGER_STAMP}
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/manager
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/manager ${CMAKE_BINARY_DIR}/manager
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/common ${CMAKE_BINARY_DIR}/manager/common
  COMMAND ${CMAKE_COMMAND} -E touch ${MANAGER_STAMP}
  VERBATIM
)

add_custom_target(
    manager ALL
    DEPENDS ${MANAGER_STAMP}
)
