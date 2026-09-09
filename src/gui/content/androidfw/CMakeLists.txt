# androidfw — independent sub-library: runtime readers for the aapt2 binary
# resource format (resources.arsc + binary AXML). A faithful C++14 port of AOSP
# frameworks/base/libs/androidfw, stages:
#   1. ResStringPool (string pool reader)            — done
#   2. ResXMLTree   (binary AXML pull parser)        — done
#   3a. ResTable_config (config matching engine)     — done
#   3b. ResTable    (arsc table load + getResource)  — RETIRED with the AM1
#       retirement (the AM2 switch replaced the engine; restable.{h,cc} deleted)
#
# Built and unit-tested in ISOLATION: registered at the top-level CMakeLists
# scope (not under src/gui), so it does NOT inherit src/gui's directory-scope
# force-includes and is NOT linked into libcdroid.so. Logging goes through
# <porting/cdlog.h> (linked via tvhal); LocaleData.{h,cpp} + LocaleDataTables.cpp
# are vendored verbatim (self-contained CLDR locale-matching, like minikin).
project(androidfw CXX)

set(ANDROIDFW_SOURCES
    resourcetypes.cc        # foundational binary-format types + ResStringPool/ResXMLTree/ResTable_config
    # restable.cc (legacy ResTable engine) is RETIRED from the product build —
    # zero production references since the AM2 switch. Source kept in-tree as
    # the reconcile-test oracle; tests/androidfw compiles it into its own exe.
    chunkiterator.cc        # AOSP ChunkIterator (LoadedArsc's chunk walker)
    loadedarsc.cc           # AM2 stage 1: read-only validated arsc view (LoadedPackage/TypeSpec)
    zerocopyzip.cc          # AM2 stage 2: mmap'd whole-pak read header (STORED view / DEFLATED inflate)
    assetsprovider.cc       # AM2 stage 2: AOSP AssetsProvider family on the read header
    apkassets.cc            # AM2 stage 2: ApkAssets (provider + LoadedArsc)
    assetmanager2.cc        # AM2 stage 4: AssetManager2 + Theme (bag cache, filter list)
    attributeresolution.cc  # AM2 stage 4: ResolveAttrs/ApplyStyle/RetrieveAttributes bodies
    LocaleData.cpp          # vendored locale matching engine (CLDR tables)
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
           ${CMAKE_CURRENT_SOURCE_DIR}/..   # src/gui/content
           ${CMAKE_CURRENT_SOURCE_DIR}/../..   # src/gui: for <porting/cdlog.h> etc.
           ${CMAKE_BINARY_DIR}/include
           PRIVATE ${ZIP_INCLUDE_DIRS})
target_link_libraries(androidfw PUBLIC tvhal ${ZIP_LIBRARIES} ZLIB::ZLIB)
target_compile_options(androidfw PRIVATE -Wall -Wextra -Werror)
# Asset public API uses off64_t/lseek64 (AOSP-faithful); expose LFS to consumers too.
target_compile_definitions(androidfw PUBLIC _LARGEFILE64_SOURCE)

# The stage-by-stage verification programs (bare-main *_test.cc per feature,
# three closed-loop demos, in-tree byte fixtures, generated testdata/) that
# scaffolded the port were retired — they had fallen out of every build path
# after the move to content/ (only the OBJECT library ever compiled). Coverage
# lives in tests/androidfw (the gtest suite; axml_fixture.h is still its data).
# Recoverable from git history should the xmlparser split need them again.
