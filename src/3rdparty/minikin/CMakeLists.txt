cmake_minimum_required(VERSION 3.22)

project(minikin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(minikin)
target_sources(minikin
	PRIVATE
        libs/minikin/BidiUtils.cpp
        libs/minikin/FontFamily.cpp
        libs/minikin/Hyphenator.cpp
        libs/minikin/LayoutUtils.cpp
        libs/minikin/LocaleListCache.cpp
        libs/minikin/OptimalLineBreaker.cpp
        libs/minikin/CmapCoverage.cpp
        libs/minikin/FontUtils.cpp
        libs/minikin/HyphenatorMap.cpp
        libs/minikin/LineBreaker.cpp
        libs/minikin/MeasuredText.cpp
        libs/minikin/SparseBitSet.cpp
        libs/minikin/Emoji.cpp
        libs/minikin/GraphemeBreak.cpp
        libs/minikin/LayoutCore.cpp
        libs/minikin/LineBreakerUtil.cpp
        libs/minikin/Measurement.cpp
        libs/minikin/SystemFonts.cpp
        libs/minikin/FontCollection.cpp
        libs/minikin/GreedyLineBreaker.cpp
        libs/minikin/Layout.cpp
        libs/minikin/Locale.cpp
        libs/minikin/MinikinInternal.cpp
        libs/minikin/WordBreaker.cpp


		libs/unicode/uchar.cpp
		libs/unicode/ubidi.cpp
		libs/unicode/ubrk.cpp
		libs/unicode/uscript.cpp
		libs/unicode/uloc.cpp
		libs/unicode/unorm2.cpp
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
)
#find_package(HarfBuzz REQUIRED)
target_include_directories(minikin
	PRIVATE
        libs
		${PROJECT_SOURCE_DIR}/include
        ${HARFBUZZ_INCLUDE_DIRS}
        BEFORE
)
target_link_libraries(minikin
	PRIVATE
        ${HARFBUZZ_LIBRARIES}
)

enable_testing()
if(0)
add_executable(test_ubrk test_ubrk.cpp)
target_include_directories(test_ubrk PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(test_ubrk PRIVATE minikin)

add_test(NAME test_ubrk COMMAND test_ubrk)

add_executable(test_ubidi test_ubidi.cpp)
target_include_directories(test_ubidi PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(test_ubidi PRIVATE Minikin)

add_test(NAME test_ubidi COMMAND test_ubidi)
endif()

add_executable(test_minikin_cairo test_minikin_cairo.cpp)
target_include_directories(test_minikin_cairo PRIVATE libs ${PROJECT_SOURCE_DIR}/include /usr/include/freetype2 /usr/include/harfbuzz /usr/include/cairo)
target_link_libraries(test_minikin_cairo PRIVATE minikin freetype harfbuzz cairo)

add_test(NAME test_minikin_cairo COMMAND test_minikin_cairo)

