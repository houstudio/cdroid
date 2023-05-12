cmake_minimum_required(VERSION 3.11)

project(litebrowser CXX)


if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG -g")
set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELEASE "-O3")

find_package(PkgConfig REQUIRED)
find_package(litehtml CONFIG)
find_package(CURL CONFIG)

#pkg_check_modules(LB_LIBS REQUIRED fontconfig gdkmm-3.0 gtkmm-3.0 libcurl cairo pango pangocairo)
#pkg_check_modules(LB_LIBS REQUIRED curl cairo

set(SOURCE  
     src/htmlview.cc
     src/webhistory.cc
     containers/linux/container_linux.cpp
     #containers/cairo/cairo_container.cpp
     #containers/cairo/cairo_font.cpp
     #main.cpp
)

set(HEADERS 
     #${LITEBROWSER_PATH}/browser_wnd.h
     src/htmlview.h
     src/webhistory.h
     containers/linux/container_linux.h
)

#set(LITEHTML_INCLUDE_DIRS "/home/houzh/vcpkg/installed/x64-linux-dynamic/include/litehtml")

add_library(${PROJECT_NAME} ${SOURCE} ${HEADERS})
include_directories(
   ./ src containers 
   ${LITEHTML_INCLUDE_DIRS} 
   ${LB_LIBS_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/cairo
   ${CMAKE_BINARY_DIR}/include/epg
   ${CMAKE_BINARY_DIR}/include/freetype2	
)
#link_directories(/home/houzh/vcpkg/installed/x64-linux-dynamic/debug/lib)
target_link_options(${PROJECT_NAME} PRIVATE ${LB_LIBS_LDFLAGS})
target_link_libraries(${PROJECT_NAME} litehtml )#CURL::libcurl ${LB_LIBS_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES    CXX_STANDARD 11  C_STANDARD 99)

add_executable(htmltest main.cpp)
target_link_libraries(htmltest litebrowser cdroid)
