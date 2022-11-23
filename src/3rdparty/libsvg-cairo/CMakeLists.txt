project(C CXX)
aux_source_directory(src SRCS_CAIRO_SVG)
find_Package(Cairo)

message(FATAL_ERROR "CAIRO_INCLUDE_DIRS=${CAIRO_INCLUDE_DIR}")
include_directories(include src
	${CAIRO_INCLUDE_DIRS}/cairo
	${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg/src
	)
set(CMAKE_C_FLAGS "-fPIC")
#configure_file(src/svg-cairo-version.h.in ${PROJECT_SOURCE_DIR}/svg-cairo.version.h) 
add_library(svg-cairo  ${SRCS_CAIRO_SVG})
target_link_libraries(svg-cairo svg)

set(HEADERS include/svg.h
    src/svg-cairo.h
)
file(COPY ${HEADERS} DESTINATION ${CMAKE_BINARY_DIR}/include)
install(TARGETS svg-cairo DESTINATION lib)
install(FILES ${HEADERS} DESTINATION  include)
