if (EXISTS "${CMAKE_SOURCE_DIR}/src/gui/widget/view.h" )
   set(CDROID_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src/gui)
else()
   set(CDROID_INCLUDE_DIR  ${CMAKE_BINARY_DIR}/include)
endif()
#message(FATAL_ERROR "CDROID_INCLUDE_DIR=${CDROID_INCLUDE_DIR}")
set(CDROID_INCLUDE_DIRS ${CDROID_INCLUDE_DIR})
set(CDROID_LIBRARY gui)
set(CDROID_LIBRARIES ${CDROID_LIBRARY})
set(CDROID_INCLUDE_DIRS ${CDROID_INCLUDE_DIR})
set(CDROID_FOUND TRUE)
