cmake_minimum_required(VERSION 3.22)

project(minikin4)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(minikin)

set(MYICU_SOURCES
       libs/unicode/uchar.cpp
       libs/unicode/ubidi.cpp
       libs/unicode/ubrk.cpp
       libs/unicode/uscript.cpp
       libs/unicode/uloc.cpp
       libs/unicode/unicode_casemap_nfd.cpp
       libs/unicode/unicode_data_props.cpp
       libs/unicode/unorm2.cpp
   )
target_sources(minikin PRIVATE
    ${MYICU_SOURCES}
    libs/minikin/BidiUtils.cpp
    libs/minikin/BoundsCache.cpp
    libs/minikin/CmapCoverage.cpp
    libs/minikin/Emoji.cpp
    libs/minikin/FontCollection.cpp
    libs/minikin/Font.cpp
    libs/minikin/FontFamily.cpp
    libs/minikin/FontFeatureUtils.cpp
    #libs/minikin/FontFileParser.cpp
    libs/minikin/FontUtils.cpp
    libs/minikin/GraphemeBreak.cpp
    libs/minikin/GreedyLineBreaker.cpp
    libs/minikin/Hyphenator.cpp
    libs/minikin/HyphenatorMap.cpp
    libs/minikin/LayoutCore.cpp
    libs/minikin/Layout.cpp
    libs/minikin/LayoutUtils.cpp
    libs/minikin/LineBreaker.cpp
    libs/minikin/LineBreakerUtil.cpp
    libs/minikin/Locale.cpp
    libs/minikin/LocaleListCache.cpp
    libs/minikin/MeasuredText.cpp
    libs/minikin/Measurement.cpp
    libs/minikin/MinikinFontFactory.cpp
    libs/minikin/MinikinInternal.cpp
    libs/minikin/OptimalLineBreaker.cpp
    libs/minikin/SparseBitSet.cpp
    libs/minikin/SystemFonts.cpp
    libs/minikin/WordBreaker.cpp
PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include
)

if(TRUE OR NOT USE_ICU)
   target_include_directories(minikin PRIVATE
       libs
       ${PROJECT_SOURCE_DIR}/include
       ${PROJECT_SOURCE_DIR}/include/myicu
       ${HARFBUZZ_INCLUDE_DIRS}
       ${FRIBIDI_INCLUDE_DIRS}
       ${CMAKE_BINARY_DIR}/include/gui
       ${CMAKE_BINARY_DIR}/include/porting
       BEFORE
   )
   target_link_libraries(minikin PRIVATE ${HARFBUZZ_LIBRARIES} ${FRIBIDI_LIBRARIES})
else()
    find_package(ICU COMPONENTS uc REQUIRED)
    target_include_directories(minikin PRIVATE
        ${PROJECT_DSOURCE_DIR}/libs
		${PROJECT_SOURCE_DIR}/include
        ${HARFBUZZ_INCLUDE_DIRS}
        BEFORE
    )
    target_link_libraries(minikin PRIVATE  ICU::uc)
    message(WARNING "USE ICU ${ICU_VERSION}")
endif()

