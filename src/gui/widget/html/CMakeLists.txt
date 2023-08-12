
project(litebrowser CXX)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(PkgConfig REQUIRED)
find_package(litehtml CONFIG)
find_package(CURL CONFIG)

#pkg_check_modules(LB_LIBS REQUIRED curl cairo

set(SOURCE  
     src/htmlview.cc
     src/webhistory.cc
     containers/linux/container_linux.cpp
)

set(HEADERS 
     src/htmlview.h
     src/webhistory.h
     containers/linux/container_linux.h
)

if(litehtml_FOUND)
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

   target_link_options(${PROJECT_NAME} PRIVATE ${LB_LIBS_LDFLAGS})
   target_link_libraries(${PROJECT_NAME} litehtml )#CURL::libcurl ${LB_LIBS_LIBRARIES})
   set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11  C_STANDARD 99)
   add_executable(htmltest main.cpp)
   target_link_libraries(htmltest litebrowser cdroid)
endif()
