project(svg C CXX)

aux_source_directory(./src SRCS_SVG)
set(CMAKE_BUILD_TYPE Debug)
include_directories(./ ./src
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/3rdparty/uriparser/include
)

set(CMAKE_C_FLAGS "-DHAVE_LIBCURL=1 -fPIC")

add_library(svg STATIC ${SRCS_SVG})
target_link_libraries(svg expat curl uriparser png)

install(TARGETS svg DESTINATION lib)

