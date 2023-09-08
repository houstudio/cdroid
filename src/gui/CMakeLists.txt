project(cdroid C CXX)

set(CMAKE_CXX_STANDARD 11)

option( GUI_STATIC "UI is static link" ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")

aux_source_directory(core SRCS_GUICORE)
aux_source_directory(view SRCS_VIEWS)
aux_source_directory(widget SRCS_WIDGET)
aux_source_directory(ext_widget SRCS_EXTWIDGET)
aux_source_directory(drawables SRCS_DRAWABLES)
aux_source_directory(cairomm   SRCS_CAIROMM)
aux_source_directory(app SRCS_APP)
aux_source_directory(animation SRCS_ANIMATION)
aux_source_directory(private SRCS_PRIVATE)
aux_source_directory(utils SRCS_UTILS)
include_directories(
    ./ ./private 
    ./core 
    ./view 
    ./cairomm 
    ./core/svg 
    ${CMAKE_BINARY_DIR}/include
    ${CDROID_DEPINCLUDES}
    ${CMAKE_BINARY_DIR}/include/gui
    ${CMAKE_BINARY_DIR}/include/porting
    ${CMAKE_BINARY_DIR}/include/pinyin
    ${LIBUNIBREAK_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/3rdparty/pinyin/include
)

add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)

set(SOURCES_GUI ${SRCS_GUICORE} ${SRCS_VIEWS} 
    ${SRCS_CAIROMM} ${SRCS_WIDGET}
    ${SRCS_EXTWIDGET} ${SRCS_DRAWABLES}
    ${SRCS_APP} ${SRCS_DIALOG}
    #${SRCS_PRIVATE}
    ${SRCS_UTILS}
    ${SRCS_ANIMATION}
)

add_library(cdroid SHARED ${SOURCES_GUI})
CreatePAK(cdroid ${PROJECT_SOURCE_DIR}/res  ${PROJECT_BINARY_DIR}/cdroid.pak ${PROJECT_SOURCE_DIR}/widget/R.h)

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

configure_file(
        "cdroid.pc.in"
        "cdroid.pc"
        @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cdroid.pc"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")

set(PKGCONFIG_LIBS "cairo bzip2 libzip")#do not use -l
set(PKGCONFIG_LIBS_PRIVATE "expat bzip2 libzip")
set(NONPKG_LIBS "-l${PROJECT_NAME} -ltvhal")#non pkgconfig libraries
set(NONPKG_LIBS_PRIVATE "-ltvhal")
message("==CDROID_DEPLIBS=${CDROID_DEPLIBS}")
target_link_libraries(cdroid ${CDROID_DEPLIBS} tvhal)

if(litehtml_FOUND)
   add_subdirectory(widget/html)
endif()

configure_file(cdroid.pc.in cdroid.pc @ONLY)
install(TARGETS cdroid  DESTINATION lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroid.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
install(FILES ${CDROID_DEPLIBS} DESTINATION lib)
install (DIRECTORY ${PROJECT_SOURCE_DIR} DESTINATION include/  FILES_MATCHING PATTERN "*.h"
    PATTERN "view" EXCLUDE 
    PATTERN "core" EXCLUDE)

