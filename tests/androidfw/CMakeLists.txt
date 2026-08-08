# Ported AOSP androidfw GTest suite — validates our androidfw port against
# AOSP's own test expectations. Links the androidfw static lib (not cdroid).
project(androidfw_test CXX)

file(GLOB SRCS CONFIGURE_DEPENDS *.cpp)

add_executable(androidfw_test ${SRCS})
target_include_directories(androidfw_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/gui/androidfw
    ${GTEST_INCLUDE_DIRS})
target_link_libraries(androidfw_test PRIVATE androidfw ${GTEST_LIBRARIES})

install(TARGETS androidfw_test DESTINATION bin/tests)
