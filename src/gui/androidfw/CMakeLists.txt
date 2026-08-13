# androidfw — independent sub-library: runtime readers for the aapt2 binary
# resource format (resources.arsc + binary AXML). A faithful C++14 port of AOSP
# frameworks/base/libs/androidfw, stages:
#   1. ResStringPool (string pool reader)            — done
#   2. ResXMLTree   (binary AXML pull parser)        — done
#   3a. ResTable_config (config matching engine)     — done (this stage)
#   3b. ResTable    (arsc table load + getResource)  — later
#
# Built and unit-tested in ISOLATION: registered at the top-level CMakeLists
# scope (not under src/gui), so it does NOT inherit src/gui's directory-scope
# force-includes and is NOT linked into libcdroid.so. Logging goes through
# <porting/cdlog.h> (linked via tvhal); LocaleData.{h,cpp} + LocaleDataTables.cpp
# are vendored verbatim (self-contained CLDR locale-matching, like minikin).
project(androidfw CXX)

set(ANDROIDFW_SOURCES
    resourcetypes.cc        # foundational binary-format types + ResStringPool/ResXMLTree/ResTable_config
    restable.cc             # ResTable engine (arsc load/getResource) + obtainStyledAttributes
    LocaleData.cpp          # vendored locale matching engine (CLDR tables)
    typedvalue.cc           # TypedValue (AOSP android.util.TypedValue) + applyDimension
)

# libzip is the zip backend for AssetManager (replaces AOSP ZipFileRO/libziparchive);
# zlib is used directly by asset.cc for gzip inflate. ZIP is already found project-wide
# (cmake/options.cmake); find again here so this module is self-contained.
find_package(ZIP REQUIRED)
find_package(ZLIB REQUIRED)

# OBJECT library: androidfw's compiled objects are linked directly into cdroid.so
# (cdroid target links `androidfw`) AND reused by the standalone unit-test targets
# below — compiled once, shared. This makes androidfw "participate in cdroid.so as
# source" without a separate .a archive. POSITION_INDEPENDENT_CODE is required so the
# same objects can link into the cdroid SHARED lib.
add_library(androidfw OBJECT ${ANDROIDFW_SOURCES})
set_target_properties(androidfw PROPERTIES POSITION_INDEPENDENT_CODE ON)
# Own headers (PUBLIC so consumers/tests find resourcetypes.h / assetmanager.h) +
# the generated porting include tree for <porting/cdlog.h>. Link tvhal (PUBLIC) so
# the cdlog LogMessage backend is pulled in for this lib and for the test targets.
target_include_directories(androidfw
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
           ${CMAKE_CURRENT_SOURCE_DIR}/..   # src/gui: for <core/displaymetrics.h> (TypedValue)
           ${CMAKE_BINARY_DIR}/include
           PRIVATE ${ZIP_INCLUDE_DIRS})
target_link_libraries(androidfw PUBLIC tvhal ${ZIP_LIBRARIES} ZLIB::ZLIB)
target_compile_options(androidfw PRIVATE -Wall -Wextra -Werror)
# Asset public API uses off64_t/lseek64 (AOSP-faithful); expose LFS to consumers too.
target_compile_definitions(androidfw PUBLIC _LARGEFILE64_SOURCE)

# --- Unit tests (no GUI / runtime deps; link the static lib) ---
enable_testing()
add_executable(androidfw_resourcetypes_test resourcetypes_test.cc)
target_link_libraries(androidfw_resourcetypes_test PRIVATE androidfw)
add_test(NAME resourcetypes COMMAND androidfw_resourcetypes_test)

add_executable(androidfw_resxmltree_test resxmltree_test.cc)
target_link_libraries(androidfw_resxmltree_test PRIVATE androidfw)
add_test(NAME resxmltree COMMAND androidfw_resxmltree_test)

add_executable(androidfw_config_test config_test.cc)
target_link_libraries(androidfw_config_test PRIVATE cdroid)
add_test(NAME restable_config COMMAND androidfw_config_test)

add_executable(androidfw_dynamicreftable_test dynamicreftable_test.cc)
target_link_libraries(androidfw_dynamicreftable_test PRIVATE androidfw)
add_test(NAME dynamicreftable COMMAND androidfw_dynamicreftable_test)

# Multi-package: two 0x7f arscs, one as shared lib -> id remapped + DynamicRefTable.
add_executable(androidfw_multipackage_test multipackage_test.cc)
target_link_libraries(androidfw_multipackage_test PRIVATE androidfw)
add_test(NAME multipackage COMMAND androidfw_multipackage_test)

# Theme: applyStyle with parent inheritance + force/clear semantics.
add_executable(androidfw_theme_test theme_test.cc)
target_link_libraries(androidfw_theme_test PRIVATE androidfw)
add_test(NAME theme COMMAND androidfw_theme_test)

# Full TypedArray: applyStyle priority merge (element > style=) + typed getters.
add_executable(androidfw_full_typedarray_test full_typedarray_test.cc)
target_link_libraries(androidfw_full_typedarray_test PRIVATE androidfw)
add_test(NAME full_typedarray COMMAND androidfw_full_typedarray_test)

# AssetManager2 management: getIdentifier / getResourceName / listPackageNames / getConfigurations.
add_executable(androidfw_mgmt_test mgmt_test.cc)
target_link_libraries(androidfw_mgmt_test PRIVATE androidfw)
add_test(NAME mgmt COMMAND androidfw_mgmt_test)

# The canonical ResTable test: traverse every resource in an arsc and output its
# full info (id, type/key, value or bag entries), with config-selection + bag
# fidelity assertions. Replaces the former narrow per-feature tests.
add_executable(androidfw_traverse_test traverse_test.cc)
target_link_libraries(androidfw_traverse_test PRIVATE cdroid)
add_test(NAME traverse COMMAND androidfw_traverse_test)

# Legacy AssetManager: addAssetPath/open/openNonAsset/openDir + ResTable wiring,
# exercising real apks in testdata/_gen.
add_executable(androidfw_assetmanager_test assetmanager_test.cc)
target_link_libraries(androidfw_assetmanager_test PRIVATE cdroid)
target_compile_definitions(androidfw_assetmanager_test PRIVATE
    ANDROIDFW_TESTDATA="${CMAKE_CURRENT_SOURCE_DIR}/testdata")
add_test(NAME assetmanager COMMAND androidfw_assetmanager_test)

# Resources: ID-based value/meta/asset resolution over an AssetManager. Now
# links cdroid (ResourcesImpl moved to core/, the cairo-aware cdroid layer, so
# it can own the GUI factories + caches). Uses the arsc testdata apk.
add_executable(androidfw_resources_test resources_test.cc)
target_link_libraries(androidfw_resources_test PRIVATE cdroid)
target_compile_definitions(androidfw_resources_test PRIVATE
    ANDROIDFW_TESTDATA="${CMAKE_CURRENT_SOURCE_DIR}/testdata")
add_test(NAME resources COMMAND androidfw_resources_test)

# Closed-loop demo: dump a binary AXML tree (fixture or any file arg), like
# `aapt2 dump xmltree`. Verifies the parser end to end.
add_executable(androidfw_axml_dump androidfw_axml_dump.cc)
target_link_libraries(androidfw_axml_dump PRIVATE androidfw)

# Closed-loop demo: resolve a string across locales and an integer across
# densities from a real arsc — the multi-language / multi-resolution verification.
add_executable(androidfw_resource_demo androidfw_resource_demo.cc)
target_link_libraries(androidfw_resource_demo PRIVATE androidfw)

# Demo: a minimal TypedArray running on the current state (ResXMLTree only) —
# proves aapt2-typed attrs can be read with the Android TypedArray getter shape.
add_executable(androidfw_typedarray_demo androidfw_typedarray_demo.cc)
target_link_libraries(androidfw_typedarray_demo PRIVATE androidfw)
