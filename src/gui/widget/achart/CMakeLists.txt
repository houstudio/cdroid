project(achart C CXX)

include_directories(
    ../../
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/gui
    ${CAIRO_INCLUDE_DIRS}
)

add_executable(acharttest main.cc)
target_link_libraries(acharttest cdroid)
