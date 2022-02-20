set(FTFIND_ARGS ${CMAKE_SOURCE_DIR}/src/3rdparty/freetype-2.11.0)
find_path(
  FT_INCLUDE_DIR_freetype2
  NAMES
    freetype/config/ftheader.h
    config/ftheader.h
  PATHS  ${FTFIND_ARGS}
  NO_DEFAULT_PATH
  PATH_SUFFIXES
    include/freetype2
    include
    freetype2
)
unset(FTFIND_ARGS)
message("FTFIND_ARGS=${FTFIND_ARGS} FT_INCLUDE_DIR_freetype2=${FT_INCLUDE_DIR_freetype2}")
set(FREETYPE_INCLUDE_DIR  ${FT_INCLUDE_DIR_freetype2})
set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIR})
set(FREETYPE_LIBRARY freetype)
set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY})
set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIR})
set(FREETYPE_FOUND TRUE)
