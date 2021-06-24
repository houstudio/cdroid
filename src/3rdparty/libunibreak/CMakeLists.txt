
project(C CXX)

aux_source_directory(src SRCS_UNIBREAK)

list(REMOVE_ITEM SRCS_UNIBREAK src/emojidata.c) #emojidata.c is included in emojidef.c

include_directories(./ src)
add_library(unibreak STATIC ${SRCS_UNIBREAK})

#graphemebreak_test.c  linebreak_test.c  test.txt  wordbreak_test.c
#add_executable(linebreak_test tools/linebreak_test.c)
#target_link_libraries(linebreak_test unibreak)
set(CMAKE_C_FLAGS -fPIC)

#add_executable(graphemebreak_test tools/graphemebreak_test.c)
#target_link_libraries(graphemebreak_test unibreak)

#add_executable(wordbreak_test tools/wordbreak_test.c)
#target_link_libraries(wordbreak_test unibreak)

install(TARGETS unibreak DESTINATION lib)
#install(TARGETS linebreak_test DESTINATION bin)
#install(TARGETS graphemebreak_test DESTINATION bin)
#install(TARGETS wordbreak_test DESTINATION bin)



