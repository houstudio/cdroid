project(gui C CXX)

set(CMAKE_CXX_STANDARD 11)

option( LUA_BINDINGS "UI Lua script bindings" OFF)
option( GUI_STATIC "UI is static link" ON)

aux_source_directory(core SRCS_GUICORE)
aux_source_directory(view SRCS_VIEWS)
aux_source_directory(widget SRCS_WIDGET)
aux_source_directory(drawables SRCS_DRAWABLES)
aux_source_directory(cairomm   SRCS_CAIROMM)
aux_source_directory(app SRCS_APP)
aux_source_directory(animation SRCS_ANIMATION)
find_package(PNG)
find_package(JPEG)
find_package(ZIP)
find_package(Freetype)
find_package(EXPAT)
find_package(PLPLOT)
find_package(Fontconfig)

include_directories(
    ./ ./private 
    ./core 
    ./view 
    ./cairomm 
    ./core/svg ./luabinding 
    ${CMAKE_BINARY_DIR}/include
    ${PNG_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIR} ${PLPLOT_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/include/gui
    ${CMAKE_BINARY_DIR}/include/pixman
    ${CMAKE_BINARY_DIR}/include/cairo
    ${FREETYPE_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include/porting
    ${CMAKE_BINARY_DIR}/include/pinyin
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libunibreak/src
    ${CMAKE_SOURCE_DIR}/src/3rdparty/pinyin/include
    ${CMAKE_BINARY_DIR}/include/fribidi
    ${CMAKE_SOURCE_DIR}/src/modules/vold
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg/src
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/src
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/include
)

add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)
set(CDROID_LIBS 
	#${PIXMAN_LIBRARIES} ${CAIRO_LIBRARIES}
	${FONTCONFIG_LIBRARIES}
        ${ZIP_LIBRARIES}	
	${PNG_LIBRARIES} ${JPEG_LIBRARIES}
	${JSONCPP_LIBRARIES} ${EXPAT_LIBRARIES}
	${ZIP_LIBRARIES}
	)

set(SOURCES_GUI ${SRCS_GUICORE} ${SRCS_VIEWS} 
    ${SRCS_CAIROMM} ${SRCS_WIDGET} ${SRCS_DRAWABLES} 
    ${SRCS_APP} ${SRCS_DIALOG} 
    ${SRCS_ANIMATION} widget/R.h)

message("FONTCONFIG_LIBRARIES=${FONTCONFIG_LIBRARIES} CDROID_LIBS=${CDROID_LIBS}")

add_library(cdroid_objs OBJECT ${SOURCES_GUI})
add_library(cdroid_static STATIC $<TARGET_OBJECTS:cdroid_objs>)
add_library(cdroid SHARED $<TARGET_OBJECTS:cdroid_objs>)
CreatePAK(cdroid ${PROJECT_SOURCE_DIR}/res  ${PROJECT_BINARY_DIR}/cdroid.pak ${PROJECT_SOURCE_DIR}/widget/R.h)

set_target_properties(cdroid_static PROPERTIES OUTPUT_NAME "cdroid")

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

target_link_libraries(cdroid 
	PRIVATE cairo png jpeg unibreak zip jsoncpp expat
	${OPTIONAL_LIBS} 
	PUBLIC tvhal )
target_link_libraries(cdroid_static 
	PRIVATE png jpeg cairo_static unibreak  zip jsoncpp expat
	${OPTIONAL_LIBS}
	PUBLIC tvhal_static )
configure_file(
        "cdroid.pc.in"
        "cdroid.pc"
        @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cdroid.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

install (TARGETS cdroid  DESTINATION lib)
install (TARGETS cdroid_static  DESTINATION lib)
install (DIRECTORY ${PROJECT_SOURCE_DIR} DESTINATION include/  FILES_MATCHING PATTERN "*.h"
    PATTERN "view" EXCLUDE 
    PATTERN "core" EXCLUDE)
install (FILES ${LOOPERHEADERS} DESTINATION include/looper)

