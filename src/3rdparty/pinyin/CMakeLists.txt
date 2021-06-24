project(pinyin C CXX)
cmake_minimum_required(VERSION 3.5)
aux_source_directory(share PINYIN_SRCS)
add_library(pinyin ${PINYIN_SRCS})
include_directories(include)
target_link_libraries(pinyin pthread)

add_executable(dictbuild ${PINYIN_SRCS} command/pinyinime_dictbuilder.cpp)
target_link_libraries(dictbuild pthread)
target_compile_definitions(dictbuild PRIVATE ___BUILD_MODEL___)

add_executable(pytest test.cpp)
target_link_libraries(pytest pinyin)

file(GLOB PINYININC   ${PROJECT_SOURCE_DIR}/include/*.h)
file(COPY ${PINYININC} DESTINATION ${CMAKE_BINARY_DIR}/include/pinyin  PATTERN "*.h")
install(TARGETS pinyin DESTINATION lib)
