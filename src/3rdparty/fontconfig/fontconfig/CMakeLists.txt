# fontconfig
FILE(COPY fontconfig.h DESTINATION ${CMAKE_BINARY_DIR}/include/fontconfig)
install( FILES fcfreetype.h fcprivate.h fontconfig.h DESTINATION include/fontconfig )
