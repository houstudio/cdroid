project( fontconfig )
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

find_package( EXPAT REQUIRED )
find_package( Freetype REQUIRED )

include_directories(
    ${EXPAT_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
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
  target_link_libraries(${util} fontconfig zlib)
  install(TARGETS ${util} DESTINATION bin)
endforeach()

