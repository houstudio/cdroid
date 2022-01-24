project(gui C CXX)

set(CMAKE_CXX_STANDARD 11)

option( LUA_BINDINGS "UI Lua script bindings" OFF)
option( GUI_STATIC "UI is static link" ON)

aux_source_directory(core SRCS_GUICORE)
aux_source_directory(views SRCS_VIEWS)
aux_source_directory(widget SRCS_WIDGET)
aux_source_directory(drawables SRCS_DRAWABLES)
aux_source_directory(cairomm   SRCS_CAIROMM)
aux_source_directory(app SRCS_APP)
aux_source_directory(animation SRCS_ANIMATION)
find_package(PNG)
find_package(Freetype)
find_package(EXPAT)
find_package(PLPLOT)
message("UI Lua script bindings is ${LUA_BINDINGS} guistatic=${GUI_STATIC}")

include_directories(
    ./ ./private 
    ./core 
    ./views 
    ./cairomm 
    ./core/svg ./luabinding 
    ${CMAKE_BINARY_DIR}/include
    ${PNG_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIR} ${PLPLOT_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/include/pixman
    ${CMAKE_BINARY_DIR}/include/cairo
    ${CMAKE_BINARY_DIR}/include/porting
    ${CMAKE_BINARY_DIR}/include/pinyin
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libunibreak/src
    ${CMAKE_SOURCE_DIR}/src/3rdparty/pinyin/include
    ${CMAKE_BINARY_DIR}/include/fribidi
    ${CMAKE_SOURCE_DIR}/src/modules/vold
    ${CMAKE_SOURCE_DIR}/src/3rdparty/grt
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg/src
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/src
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/include
)

add_definitions(-DRAPIDJSON_HAS_STDSTRING)


set(SOURCES_GUI ${SRCS_GUICORE} ${SRCS_VIEWS} 
    ${SRCS_WIDGET} ${SRCS_DRAWABLES} 
    ${SRCS_CAIROMM} ${SRCS_DIALOG} 
    ${SRCS_ANIMATION} widget/R.h)

add_library(gui_objs OBJECT ${SOURCES_GUI})
add_library(gui_static STATIC $<TARGET_OBJECTS:gui_objs>)
add_library(gui SHARED $<TARGET_OBJECTS:gui_objs>)

CreatePAK(gui ${PROJECT_SOURCE_DIR}/res  ${PROJECT_BINARY_DIR}/cdroid.pak ${PROJECT_SOURCE_DIR}/widget/R.h)

set_target_properties(gui_static PROPERTIES OUTPUT_NAME "gui")

file(GLOB_RECURSE allfiles RELATIVE "${PROJECT_SOURCE_DIR}/" "*.h")
list(REMOVE_ITEM allfiles "private/*.h")
foreach( each_file ${allfiles} )
  STRING(REGEX REPLACE ".*/\(.*\)" "\\1" FILE_PATH ${each_file})
  STRING(LENGTH ${each_file} LEN0)
  STRING(LENGTH ${FILE_PATH} LEN1)
  MATH(EXPR LEN0 "${LEN0}-${LEN1}")
  STRING(SUBSTRING ${each_file} 0 ${LEN0} FILE_PATH)
  file(COPY ${PROJECT_SOURCE_DIR}/${each_file} DESTINATION "${CMAKE_BINARY_DIR}/include/gui/${FILE_PATH}")
  install(FILES  ${PROJECT_SOURCE_DIR}/${each_file} DESTINATION "${CMAKE_BINARY_DIR}/include/gui/${FILE_PATH}")
endforeach(each_file)

message("=======OPTIONAL_LIBS=${OPTIONAL_LIBS}")

target_link_libraries(gui pixman-1 png jpeg cairo unibreak zip jsoncpp tvhal ${OPTIONAL_LIBS} expat)
target_link_libraries(gui_static pixman-1 png jpeg cairo_static unibreak  zip jsoncpp tvhal_static ${OPTIONAL_LIBS})

install (TARGETS gui  DESTINATION lib)
install (TARGETS gui_static  DESTINATION lib)
install (DIRECTORY ${PROJECT_SOURCE_DIR} DESTINATION include/  FILES_MATCHING PATTERN "*.h"
    PATTERN "views" EXCLUDE 
    PATTERN "core" EXCLUDE)
install (FILES ${LOOPERHEADERS} DESTINATION include/looper)

