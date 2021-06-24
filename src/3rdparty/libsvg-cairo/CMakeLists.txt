project(C CXX)

aux_source_directory(src SRCS_CAIRO_SVG)
include_directories(include src
	${CMAKE_SOURCE_DIR}/src/3rdparty/cairo-1.16.0/src
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
