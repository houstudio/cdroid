project(cdroid C CXX)

include(cdroid.cmake)
set(CMAKE_LINK_WHAT_YOU_USE TRUE)
option( GUI_STATIC "UI is static link" ON)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
endif()
file(GLOB_RECURSE SRCS_WIDGETEX RELATIVE "${PROJECT_SOURCE_DIR}" "widgetEx/*")

if( ENABLE_I18N )
    aux_source_directory(i18n SRCS_I18N)
endif()
aux_source_directory(cairomm  SRCS_CAIROMM)
aux_source_directory(utils SRCS_UTILS)
aux_source_directory(navigation SRCS_NAVIGATION)
add_subdirectory(image-decoders)
aux_source_directory(image-decoders SRCS_IMAGEDECODERS)

GetGitVersion(CDROID)
message("VERSION=${CDROID_VERSION} ${CDROID_VERSION_MAJOR}.${CDROID_VERSION_MINOR}.${CDROID_VERSION_PATCH}
CDROID_BUILD_NUMBER=${CDROID_BUILD_NUMBER} COMMIT_ID=${CDROID_COMMITID}")

configure_file(core/build.h.cmake ${PROJECT_SOURCE_DIR}/core/build.h)
configure_file(gui_features.h.cmake  ${PROJECT_BINARY_DIR}/gui_features.h)

set(SOURCES_GUI
    ${SRCS_I18N}
    ${SRCS_CAIROMM}
    ${CDROID_SOURCES}
    ${SRCS_UTILS}
)

add_subdirectory(text)
add_library(cdroid SHARED ${SOURCES_GUI})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(--include gui_features.h)
    add_definitions(--include cdtypes.h)
    add_definitions(--include cdlog.h)
    add_definitions(--include stdpatch.h)
else ()#(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_options(cdroid PRIVATE
        --include gui_features.h
        --include cdtypes.h
        --include cdlog.h
        --include stdpatch.h
        )
endif()

target_include_directories(
    cdroid
    PRIVATE
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/i18n
    ${PROJECT_SOURCE_DIR}/private
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/view
    ${PROJECT_SOURCE_DIR}/cairomm
    ${PROJECT_SOURCE_DIR}/image-decoders
    ${PROJECT_SOURCE_DIR}/../porting/include
    ${PROJECT_SOURCE_DIR}/text/minikin/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/pinyin/include
    ${CMAKE_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/include/gui
    ${CMAKE_BINARY_DIR}/include/porting
    ${CMAKE_BINARY_DIR}/include/pinyin
    ${CDROID_DEPINCLUDES}
    ${LIBUNIBREAK_INCLUDE_DIRS}
)

CreatePAK(cdroid ${PROJECT_SOURCE_DIR}/res  ${PROJECT_BINARY_DIR}/cdroid.pak ${PROJECT_SOURCE_DIR}/widget/R.h)

file(GLOB_RECURSE allfiles RELATIVE "${PROJECT_SOURCE_DIR}/" "*.h")

SET(exclude_dirs private image-decoders)
set(filtered_files "")

foreach(file ${allfiles})
    set(abs_file "${PROJECT_SOURCE_DIR}/${file}")
    set(should_exclude FALSE)
    foreach(exclude_dir ${exclude_dirs})
        if(abs_file MATCHES "^${PROJECT_SOURCE_DIR}/${exclude_dir}/")
            set(should_exclude TRUE)
            break()
        endif()
    endforeach()
    if(NOT should_exclude)
        list(APPEND filtered_files ${file})
    endif()
endforeach()
# Re-run the header copy below whenever any tracked header changes. file(COPY) only runs at
# configure time; without this, editing a src header leaves the ${CMAKE_BINARY_DIR}/include/gui
# copy stale (apps include the copy) until you touch CMakeLists / re-run cmake by hand.
set(filtered_files_abs "")
foreach(f ${filtered_files})
    list(APPEND filtered_files_abs "${PROJECT_SOURCE_DIR}/${f}")
endforeach()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${filtered_files_abs}")
foreach( each_file ${filtered_files} )
  STRING(REGEX REPLACE ".*/\(.*\)" "\\1" FILE_PATH ${each_file})
  STRING(LENGTH ${each_file} LEN0)
  STRING(LENGTH ${FILE_PATH} LEN1)
  MATH(EXPR LEN0 "${LEN0}-${LEN1}")
  STRING(SUBSTRING ${each_file} 0 ${LEN0} FILE_PATH)
  file(COPY ${PROJECT_SOURCE_DIR}/${each_file} DESTINATION "${CMAKE_BINARY_DIR}/include/gui/${FILE_PATH}")
  install(FILES  ${PROJECT_SOURCE_DIR}/${each_file} DESTINATION "${CMAKE_BINARY_DIR}/include/gui/${FILE_PATH}")
endforeach(each_file)

# --- pkg-config (cdroid.pc) public/private link split ---
# cairo/cairomm is PUBLIC: cdroid public headers expose Cairo::Context / ImageSurface / RefPtr
# (Canvas is-a cairo_t). Everything else is PRIVATE (internal impl: minikin, image codecs, XML,
# compression, font, system libs). Keep in sync with the find_package() list in cmake/options.cmake.
set(PKGCONFIG_LIBS          "cairo")
set(PKGCONFIG_LIBS_PRIVATE  "freetype2 fontconfig harfbuzz libpng expat")
set(NONPKG_LIBS_PRIVATE     "-lm -pthread -ldl")
configure_file("cdroid.pc.in"  "cdroid.pc"  @ONLY)
file(COPY ${PROJECT_BINARY_DIR}/gui_features.h DESTINATION "${CMAKE_BINARY_DIR}/include/gui/")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cdroid.pc" DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")

set(PKGCONFIG_LIBS "cairo libzip")#do not use -l
set(PKGCONFIG_LIBS_PRIVATE "expat libzip")
set(NONPKG_LIBS "-l${PROJECT_NAME} -ltvhal")#non pkgconfig libraries
set(NONPKG_LIBS_PRIVATE "-ltvhal")
message("CDROID_DEPLIBS=${CDROID_DEPLIBS}")
target_link_libraries(cdroid
    PUBLIC  ${CDROID_PUBLIC_DEPLIBS} tvhal                   # cairo + tvhal (cdlog/LogMessage — cdroid public headers use LOGD/LOGE)
    PRIVATE image-decoders cdtext ${CDROID_PRIVATE_DEPLIBS})
# Prune libcdroid.so's DT_NEEDED to only the libs it actually references. The Find*.cmake modules
# are old-style (return flat ${LIBS}, not imported targets with INTERFACE_LINK_LIBRARIES), so
# transitive sub-deps land on the link line: pixman (via cairo), z/bz2 (via zip),
# webpdecoder/sharpyuv/webpmux (via webp), harfbuzz-icu (via harfbuzz). --as-needed drops those
# from NEEDED at link time; they stay reachable transitively (e.g. cairo.so NEEDED pixman).
# This is also what makes the CMAKE_LINK_WHAT_YOU_USE "unused direct dependencies" warnings
# harmless for genuinely-transitive libs.
if(UNIX AND NOT APPLE)
    target_link_options(cdroid PRIVATE -Wl,--as-needed)
elseif(MSVC)
   set_target_properties(cdroid PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(litehtml_FOUND)
   add_subdirectory(widget/html)
endif()

install(TARGETS cdroid  DESTINATION lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroid.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

