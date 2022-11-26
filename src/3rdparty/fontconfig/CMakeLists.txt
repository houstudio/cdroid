project( fontconfig )
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

find_package( EXPAT REQUIRED )
find_package( Freetype2 REQUIRED )
find_package( Iconv)
include_directories(
    ${EXPAT_INCLUDE_DIR}
    ${FREETYPE2_INCLUDE_DIRS}
    ${CMAKE_INCLUDE_PATH}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if( MSVC )
    include_directories( win_compat )
    add_definitions( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -wd4018)
endif(MSVC)
link_directories( ${CMAKE_LIBRARY_PATH} )

include( ConfigureChecks.cmake )

add_subdirectory( fontconfig )
add_subdirectory( src )

add_subdirectory( conf.d )

set(FCUTILS fc-cat fc-scan fc-cache fc-list fc-match fc-validate fc-pattern fc-query)
foreach(util ${FCUTILS} )
  add_executable(${util} ${util}/${util}.c)
  target_link_libraries(${util} fontconfig zlib harfbuzz harfbuzz-subset)
  install(TARGETS ${util} DESTINATION bin)
endforeach()

file(READ "${PROJECT_SOURCE_DIR}/fontconfig/fontconfig.h" FC_VERSION_CONTENT)

string(REGEX MATCH   "#define[ \t]+FC_MAJOR[ \t]+([0-9]+)" _dummy "${FC_VERSION_CONTENT}")
set(VERSION_MAJOR    "${CMAKE_MATCH_1}")
string(REGEX MATCH   "#define[ \t]+FC_MINOR[ \t]+([0-9]+)" _dummy "${FC_VERSION_CONTENT}")
set(VERSION_MINOR    "${CMAKE_MATCH_1}")
string(REGEX MATCH   "#define[ \t]+FC_REVISION[ \t]+([0-9]+)" _dummy "${FC_VERSION_CONTENT}")
set(VERSION_REVISION "${CMAKE_MATCH_1}")

set(PKGCONFIG_REQUIRES "freetype2")
set(PKGCONFIG_REQUIRES_PRIVATELY "expat")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fontconfig.pc.in
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig )
