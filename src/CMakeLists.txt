message("Processing src/CMakeList.txt")

include(extern/extern.cmake)
include(rpcgen/rpcgen.cmake)
include(cli/CMakeLists.txt)
include(dmn/CMakeLists.txt)

add_subdirectory(mgmt)
add_subdirectory(xdp)
