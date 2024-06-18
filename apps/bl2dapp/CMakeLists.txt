project(bl2dapp C CXX)

find_package(blend2d CONFIG REQUIRED)

include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
)
aux_source_directory(./ DEMO_SRCS)
add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)

aux_source_directory(./ BL2DSRCS)
add_executable(bl2dapp ${BL2DSRCS})
target_link_libraries(bl2dapp PRIVATE blend2d::blend2d cdroid)
