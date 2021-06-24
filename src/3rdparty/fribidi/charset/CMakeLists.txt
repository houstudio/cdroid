IF(APPLE)
#TODO: iOS
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.6")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.6")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -mmacosx-version-min=10.6")
ENDIF()

AUX_SOURCE_DIRECTORY(. SRC_LIST)
ADD_LIBRARY(fribidi-charset OBJECT ${SRC_LIST})

INSTALL(FILES fribidi-char-sets.h fribidi-char-sets-list.h DESTINATION include)
