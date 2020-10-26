message("Processing src/extern/CMakeList.txt")

set(LIBBPF "${CMAKE_BINARY_DIR}/lib/libbpf.ready")

# ----------------------------------------------------------------------------
# Install libbpf 
add_custom_command(
  OUTPUT ${LIBBPF}
  COMMAND mkdir -p ${CMAKE_BINARY_DIR}/lib
  COMMAND DESTDIR=${CMAKE_BINARY_DIR}/lib/ make install -C ${CMAKE_CURRENT_LIST_DIR}/libbpf/src
  COMMAND cmake -E touch ${LIBBPF}
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/libbpf/src/*.c ${CMAKE_CURRENT_LIST_DIR}/libbpf/src/*.h 
  COMMENT "Install libbpf"
  VERBATIM
)

add_custom_target(
    libbpf
    DEPENDS ${LIBBPF}
)
include_directories(${CMAKE_BINARY_DIR}/lib/usr/include)

file(GLOB EXT_SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.c)
add_library(extern ${EXT_SOURCES})
add_dependencies(extern libbpf)

