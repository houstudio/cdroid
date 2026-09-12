cmake_minimum_required (VERSION 3.22)
project(CDROID C CXX)

# Binary AXML mode: when ON, pakbuilder uses aapt2 to produce binary AXML
# layouts + resources.arsc in the pak. Default OFF (text XML).
#
# aapt2 is auto-located on PATH (Ubuntu's aapt2 2.19 works with the android-34
# framework jar). No android-sdk dependency: the framework jar ships in-tree at
# src/gui/res/android.jar (jar is not in pakbuilder's BIN_EXTS, so it is never
# packed into cdroid.pak — it is a build input only).
find_program(CDROID_AAPT2_CMD aapt2)
if(NOT CDROID_AAPT2_CMD)
    set(CDROID_AAPT2_CMD "/opt/android-sdk/build-tools/36.0.0/aapt2")
endif()
set(CDROID_AAPT2 "${CDROID_AAPT2_CMD}" CACHE PATH "aapt2 binary")
set(CDROID_ANDROID_JAR "${CMAKE_SOURCE_DIR}/src/gui/res/android.jar" CACHE PATH "android.jar")
set(CDROID_SDK_RES "${CMAKE_SOURCE_DIR}/src/gui/res" CACHE PATH "framework res dir (slim base, ex-gui/assets)")
set(CDROID_SDK_RES_FILTER "${CMAKE_SOURCE_DIR}/scripts/sdk_res_filter.json" CACHE FILEPATH "JSON filter for SDK res densities/locales")
# Global build-time res overlay (AOSP static device-overlay semantics) applied
# to the shared cdroid.pak: values* files merge per (type,name) entry, other
# files shadow whole-file by relative path. Per-app overlays live in
# apps/<name>/overlay/ and build a dedicated cdroid.pak beside that app's
# binary instead. ID-defining entries (attr/public/symbols) are rejected.
set(CDROID_OVERLAY "" CACHE PATH "optional global res overlay dir applied to the shared cdroid.pak")

# i18n data (src/gui/content/i18n/i18n.dat) carried as an internal cdroid.pak resource
# (raw/i18n.dat) so DataResource::Init can read from the pak instead of a sidecar
# .dat file — lowers deployment friction (one fewer loose file to ship). Single
# source stays in src/gui/i18n/; configure-time copy mirrors the header-copy
# auto-sync pattern.
file(COPY "${CMAKE_SOURCE_DIR}/src/gui/content/i18n/i18n.dat"
     DESTINATION "${CMAKE_SOURCE_DIR}/src/gui/res/raw")

# Enable ctest at the top level so registered tests (the tests/ gtest suites)
# are discoverable via `ctest` from the build root.
enable_testing()

set(CMAKE_POLICY_DEFAULT_CMP0012 NEW)
if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
   cmake_policy(SET CMP0012 NEW)
   #make vcpkg happy:)
endif(COMMAND cmake_policy)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

#LeakSanitizer does not work under ptrace (strace, gdb ,valgrind,...)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -g")
#set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

include(cmake/depchecks.cmake)
include(cmake/common_functions.cmake)
include(cmake/options.cmake)
include(cmake/check_cdroiddeps.cmake)

add_compile_options(-Wno-deprecated-declarations)

set(DISABLE_FORCE_DEBUG_POSTFIX ON)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions(-DDEBUG) 
endif()
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
   add_link_options("LINKER:-rpath-link,${CMAKE_SOURCE_DIR}/src/porting/${CDROID_CHIPSET}/lib")
endif()
# In-tree seed for find_package(CDROID): apps/tests resolve to the build
# tree's exported public header face (<build>/include/gui, the auto-synced
# install-parity copies) instead of NOTFOUND on a fresh configure. Deliberately
# a seed, not FORCE: an explicit -DCDROID_INCLUDE_DIRS=<installed cdroid>
# enters the cache before this runs and wins — the two entry points (in-tree
# face vs installed cdroid via FindCDROID/pkg-config) never fight. The paths
# need not exist at configure time; headers are synced while building cdroid.
set(CDROID_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/include/gui" CACHE PATH "cdroid public header face (in-tree seed)")
set(CDROID_PORTING_DIRS "${CMAKE_BINARY_DIR}/include"     CACHE PATH "cdroid porting headers (in-tree seed)")

# android.net stack (wifi/ethernet/connectivity) — standalone cdnet lib,
# consumed by apps/tests; the gui integration hooks land separately.
add_subdirectory(src/net)
# android.bluetooth stack — standalone cdblue lib over BlueZ (sd-bus),
# same module-phase shape as cdnet.
add_subdirectory(src/bluetooth)
add_subdirectory(src/porting)
# androidfw must be defined before src/gui so cdroid can link it.
add_subdirectory(src/gui/content/androidfw)
add_subdirectory(src/gui)
# testkit (the Espresso port) lives under gui/app next to autotest/uiautomation,
# but stays OUT of libcdroid: its own SHARED lib, product apps don't link it.
add_subdirectory(src/gui/app/espresso)
add_subdirectory(src/3rdparty)
# pm/am device-side CLI tools (adb-install chain) — after src/gui: they link cdroid.
add_subdirectory(src/cmds)

add_subdirectory(apps)

add_subdirectory(tests)
GetGitVersion(CDROID)
string(TIMESTAMP COMPILE_TIME "%Y%m%d_%H%M%S")
set(CPACK_PACKAGE_NAME "CDROID-${COMPILE_TIME}-${CDROID_COMMITID}")
set(CPACK_PACKAGE_VERSION ${CDROID_VERSION})
set(CPACK_GENERATOR "ZIP")  # Choose the desired generator (e.g., ZIP, TGZ, NSIS, etc.)
include(CPack)

if(BUILD_DOCS)
  include(cmake/build_doxygen.cmake)
  build_doxygen()
endif()

