if(ANY_QT_DEVICE AND PLPLOT_USE_QT5)
  # Find Qt5 components that we need to link applications against
  # the plplot library (if ENABLE_DYNDRIVERS is false) and which we
  # need in any case if we want to build qt_example.
  find_package(Qt5 5.3.1 COMPONENTS Svg Gui PrintSupport)
  if(NOT(Qt5_FOUND AND Qt5_VERSION_MAJOR STREQUAL "${CORE_Qt5_VERSION_MAJOR}" AND Qt5_VERSION_MINOR STREQUAL "${CORE_Qt5_VERSION_MINOR}" AND Qt5_VERSION_PATCH STREQUAL "${CORE_Qt5_VERSION_PATCH}"))
    message(STATUS "CORE_Qt5_FOUND = ON")
    message(STATUS "CORE_Qt5_VERSION_MAJOR = ${CORE_Qt5_VERSION_MAJOR}")
    message(STATUS "CORE_Qt5_VERSION_MINOR = ${CORE_Qt5_VERSION_MINOR}")
    message(STATUS "CORE_Qt5_VERSION_PATCH = ${CORE_Qt5_VERSION_PATCH}")
    message(STATUS "Qt5_FOUND = ${Qt5_FOUND}")
    message(STATUS "Qt5_VERSION_MAJOR = ${Qt5_VERSION_MAJOR}")
    message(STATUS "Qt5_VERSION_MINOR = ${Qt5_VERSION_MINOR}")
    message(STATUS "Qt5_VERSION_PATCH = ${Qt5_VERSION_PATCH}")
    message(STATUS "WARNING: Qt5 core build-tree and install-tree inconsistency")
  endif(NOT(Qt5_FOUND AND Qt5_VERSION_MAJOR STREQUAL "${CORE_Qt5_VERSION_MAJOR}" AND Qt5_VERSION_MINOR STREQUAL "${CORE_Qt5_VERSION_MINOR}" AND Qt5_VERSION_PATCH STREQUAL "${CORE_Qt5_VERSION_PATCH}"))
endif(ANY_QT_DEVICE AND PLPLOT_USE_QT5)

# Find export files in same directory location as present file.
include(${CMAKE_CURRENT_LIST_DIR}/plplot_exports.cmake)
