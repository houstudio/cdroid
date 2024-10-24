project (win32 C CXX)

set(WIN32_SRCS
     wininput.cc
     ../common/cdlog.cc
    )

include(CheckIncludeFile)
find_package(PkgConfig)
find_package(Pixman)
include_directories(./
    ../common ../include
    ${PIXMAN_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${WIN32_INCLUDE_DIRS}
)
add_library(tvhal SHARED ${WIN32_SRCS} graph_wgl.c)
add_library(tvhal-wgl SHARED ${WIN32_SRCS} graph_wgl.c)
add_library(tvhal-gdi SHARED ${WIN32_SRCS} graph_gdi.c)

set_target_properties(tvhal PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_target_properties(tvhal-wgl PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)   
set_target_properties(tvhal-gdi PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND WIN32DEP_LIBRARIES kernel32 gdi32 opengl32 glu32)
list(APPEND WIN32PORTS tvhal tvhal-wgl tvhal-gdi)

target_link_libraries(tvhal ${WIN32DEP_LIBRARIES} ${PIXMAN_LIBRARIES})
target_link_libraries(tvhal-wgl ${WIN32DEP_LIBRARIES} ${PIXMAN_LIBRARIES})
target_link_libraries(tvhal-gdi ${WIN32DEP_LIBRARIES} ${PIXMAN_LIBRARIES})

install (TARGETS ${WIN32PORTS}  DESTINATION lib)

