include(CMakeDependentOption)
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckSymbolExists)

find_package(ZLIB REQUIRED)
find_package(ZIP REQUIRED)
find_package(BZip2 REQUIRED)
find_package(Pixman REQUIRED)
find_package(Freetype REQUIRED)
find_package(HarfBuzz)
find_package(EXPAT REQUIRED)
find_package(Cairo REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(UniBreak REQUIRED)
find_package(RTAUDIO)
find_package(Pthread)
find_package(GIF)
find_package(JPEG)
find_package(WEBP)
find_package(OpenJPEG)
find_package(LCMS2)
find_package(Iconv)
#find_package(litehtml CONFIG)
#find_package(zint CONFIG) #barcode generater

option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_APPS "Build cdroid custom apps" ON)
option(BUILD_CDROID_TESTS "Build unit tests" ON)
option(ENABLE_GESTURE "Enable Gesture support" ON)
option(ENABLE_QRCODE "Enable QRCode(only support QRCode)" ON)
option(ENABLE_PINYIN2HZ "Chinese Pinyin to HZ support" ON)
option(ENABLE_DIALOGS "Enable AlertDialogs" ON)
option(ENABLE_SPINNER "Enable Spinner" ON)
option(ENABLE_DAYTIME_WIDGETS "Enable Daytime widgets(Experience)" ON)
option(ENABLE_RECYCLERVIEW "Enable RecyclerView" ON)
option(ENABLE_NAVIGATION "Enable Navigation" ON)
option(ENABLE_LIFECYCLE "Enable Lifecycle/ViewModel foundation" ON)
option(ENABLE_SAVEDSTATE "Enable SavedStateRegistry foundation" ON)
option(ENABLE_FRAGMENT "Enable Fragment framework" ON)
option(ENABLE_TRANSITION "Enable Transition scene animation" ON)
option(ENABLE_FLEXBOXLAYOUT "Enable FlexboxLayout(use ConstraintLayout instead)" OFF)
option(ENABLE_COORDINATORLAYOUT "Enable CoordinatorLayout(use MotionLayout instead)" OFF)
option(ENABLE_CONSTRAINTLAYOUT "Enable ConstraintLayout(contains MotionLayout)" ON)
option(ENABLE_I18N "Enable I18N" OFF)
option(ENABLE_MENU "Enable MENU(Experience)" ON)
option(ENABLE_WEARABLE_WIDGETS "Enable wearable Widgets(Experience)" ON)
option(ENABLE_KPLOT "Enable QT KPlot " ON)
option(ENABLE_ACHART "Enable AChartEngine " ON)

cmake_dependent_option(ENABLE_GIF "enable gif encode and decoder" ON "GIF_FOUND" OFF)
cmake_dependent_option(ENABLE_JPEG "enable jpeg decoder" ON "JPEG_FOUND" OFF)
cmake_dependent_option(ENABLE_WEBP "enable webp decoder" ON "WEBP_FOUND" OFF)
cmake_dependent_option(ENABLE_OPENJPEG "enable openjpeg(jpeg2000)" ON "OPENJPEG_FOUND" OFF)
cmake_dependent_option(ENABLE_AUDIO "Enabled Audio(Sound Effect)" ON "RTAUDIO_FOUND" OFF)
cmake_dependent_option(ENABLE_BARCODE "Enable BarCode(QrCode Code11 Code49 Code93...)" OFF "zint_FOUND" OFF)
cmake_dependent_option(ENABLE_LOTTIE "Enable Lottie Animation" ON "cmake_dependent_option" OFF)
cmake_dependent_option(ENABLE_LCMS "Enable Little CMS (a color management engine)" OFF "LCMS2_FOUND" OFF)

# --- Public vs private link split (drives target_link_libraries + cdroid.pc) ---
# PUBLIC: exposed in cdroid's public headers → consumers compile/link against them too.
# cairo/cairomm: Canvas is-a cairo_t; View/Drawable headers expose Cairo::Context/ImageSurface/RefPtr.
list(APPEND CDROID_PUBLIC_DEPLIBS  ${CAIRO_LIBRARIES} ${PIXMAN_LIBRARIES})
# PRIVATE: internal only. cdroid is a shared library → link-time only, not in consumer Cflags.
# (pixman/z/bz2/harfbuzz-icu/webpdecoder are transitive sub-deps of cairo/zip/harfbuzz/webp;
#  --as-needed on cdroid drops them from DT_NEEDED. Kept on the link line for static fallback.)
list(APPEND CDROID_PRIVATE_DEPLIBS
    ${FREETYPE_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${HARFBUZZ_LIBRARIES}
    ${PNG_LIBRARIES}
    ${ZIP_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${UNIBREAK_LIBRARIES}
    ${Iconv_LIBRARIES}
)

if(RTAUDIO_FOUND)
    list(APPEND CDROID_PRIVATE_DEPLIBS ${RTAUDIO_LIBRARIES})
endif()
if(MSVC)
    message(FATAL_ERRPR "PTHREAD=${PTHREAD_LIBRARIES}")
    list(APPEND CDROID_PRIVATE_DEPLIBS ${PTHREAD_LIBRAIRES} kernel32 gdi32 ws2_32)
endif()
if(ENABLE_LOTTIE)
    list(APPEND CDROID_PRIVATE_DEPLIBS ${RLOTTIE_LIBRARIES})
endif()

if (litehtml_FOUND)
    list( APPEND CDROID_PRIVATE_DEPLIBS litehtml)
    #list(APPEND CDROID_DEPINCLUDES ${LITEHTML_INCLUDE_DIRS})
    #add_definitions(-DENABLE_LITEHTML=1)
endif()

if (PLPLOT_FOUND)
    list( APPEND CDROID_PRIVATE_DEPLIBS ${PLPLOT_LIBRARIES})
    list(APPEND CDROID_DEPINCLUDES ${PLPLOT_INCLUDE_DIRS})
    add_definitions(-DENABLE_PLPLOT=1)
endif()

if (ENABLE_BARCODE)
    list( APPEND CDROID_PRIVATE_DEPLIBS zint::zint)
endif()

if(ENABLE_MATHGL)
    list(APPEND CDROID_DEPINCLUDES ${MATHGL_INCLUDE_DIRS})
    list(APPEND CDROID_PRIVATE_DEPLIBS ${MATHGL_LIBRADIES})
endif()

list(APPEND CDROID_DEPINCLUDES
    ${ZIP_INCLUDE_DIRS}
    ${EXPAT_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${HARFBUZZ_INCLUDE_DIRS}
)

message("CDROID_DEPLIBS=${CDROID_DEPLIBS}")

if(ENABLE_PINYIN2HZ)
    list(APPEND OPTIONAL_LIBS pinyin)
    list(APPEND CDROID_PRIVATE_DEPLIBS pinyin)
    list(APPEND CDROID_DEPINCLUDES ${CMAKE_SOURCE_DIR}/src/3rdparty/pinyin/include)
endif()

set(SKIP_INSTALL_EXPORT TRUE)
# CDROID_DEPLIBS = public + private combined (used by the install-symlinks foreach + message).
set(CDROID_DEPLIBS ${CDROID_PUBLIC_DEPLIBS} ${CDROID_PRIVATE_DEPLIBS})

if(NOT VCPKG_TARGET_TRIPLET)
foreach(lib ${CDROID_DEPLIBS})
    get_filename_component(libpath ${lib} DIRECTORY)
    set(linkdone FALSE)
    while(NOT linkdone AND EXISTS ${lib})
       execute_process( COMMAND readlink ${lib} OUTPUT_VARIABLE linkfile  OUTPUT_STRIP_TRAILING_WHITESPACE)
       get_filename_component(libname ${lib} NAME)
       get_filename_component(linkpath "${linkfile}" DIRECTORY)
       get_filename_component(linkname "${linkfile}" NAME)
       if("${linkpath}" STREQUAL "")
	       set(linkpath ${libpath})
       endif()
       if("${linkfile}" STREQUAL "")
           install(FILES ${lib} DESTINATION lib)
       else()
	   get_filename_component(fromfile ${linkfile} NAME)
	   install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
	          ${fromfile} ${libname} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/lib)")
       endif() ##("${linkfile}" STREQUAL "")
       set(lib ${linkpath}/${linkname})
       if("${linkfile}" STREQUAL "")
           set(linkdone TRUE)
       endif()
    endwhile()
endforeach(lib ${CDROID_DEPLIBS})
endif(NOT VCPKG_TARGET_TRIPLET)
