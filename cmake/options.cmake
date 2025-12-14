include(CMakeDependentOption)

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
find_package(Fribidi)
find_package(RTAUDIO)
find_package(Pthread)
find_package(GIF)
find_package(JPEG)
find_package(WEBP)
find_package(LCMS2)
find_package(Iconv)
#find_package(LUNASVG)
#find_package(MathGL)
#find_package(RLOTTIE)
#find_package(PLPLOT)
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
option(ENABLE_DAYTIME_WIDGETS "Enable Daytime widgets(Experience)" OFF)
option(ENABLE_RECYCLERVIEW "Enable RecyclerView" ON)
option(ENABLE_NAVIGATION "Enable Navigation" OFF)
#option(ENABLE_TRANSITION "Enable Transition scene animation(TODO...)" OFF)
option(ENABLE_FLEXBOXLAYOUT "Enable FlexboxLayout" OFF)
option(ENABLE_COORDINATORLAYOUT "Enable CoordinatorLayout" OFF)
option(ENABLE_I18N "Enable I18N" OFF)
option(ENABLE_MENU "Enable MENU(Experience)" OFF)
option(ENABLE_WEARABLE_WIDGETS "Enable wearable Widgets(Experience)" ON)

cmake_dependent_option(ENABLE_GIF "enable gif encode and decoder" ON "GIF_FOUND" OFF)
cmake_dependent_option(ENABLE_JPEG "enable jpeg decoder" ON "JPEG_FOUND" OFF)
cmake_dependent_option(ENABLE_WEBP "enable webp decoder" ON "WEBP_FOUND" OFF)
cmake_dependent_option(ENABLE_PLPLOT "Enable PLPLot" ON "PLPLOT_FOUND" OFF)
cmake_dependent_option(ENABLE_MATHGL "Enable MathGL" OFF "MATHGL_FOUND" OFF)
cmake_dependent_option(ENABLE_AUDIO "Enabled Audio(Sound Effect)" ON "RTAUDIO_FOUND" OFF)
cmake_dependent_option(ENABLE_BARCODE "Enable BarCode(QrCode Code11 Code49 Code93...)" OFF "zint_FOUND" OFF)
cmake_dependent_option(ENABLE_LOTTIE "Enable Lottie Animation" ON "cmake_dependent_option" OFF)
cmake_dependent_option(ENABLE_LCMS "Enable Little CMS (a color management engine)" OFF "LCMS2_FOUND" OFF)
cmake_dependent_option(ENABLE_FRIBIDI "Enable BiDi layout" ON "FRIBIDI_FOUND" OFF)
cmake_dependent_option(FT_WITH_HARFBUZZ "Improve auto-hinting of OpenType fonts." ON "HARFBUZZ_FOUND" OFF)

set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES} ${RTAUDIO_INCLUDE_DIRS}")

list(APPEND CDROID_DEPLIBS
    ${ZLIB_LIBRARIES}
    ${BZIP2_LIBRARIES}
    ${PNG_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${PIXMAN_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${UNIBREAK_LIBRARIES}
    ${ZIP_LIBRARIES}
    ${Iconv_LIBRARIES}
)

if(RTAUDIO_FOUND)
    list(APPEND CDROID_DEPLIBS ${RTAUDIO_LIBRARIES})
endif()
if(MSVC)
    message(FATAL_ERRPR "PTHREAD=${PTHREAD_LIBRARIES}")
    list(APPEND CDROID_DEPLIBS ${PTHREAD_LIBRAIRES} kernel32 gdi32 ws2_32)
endif()
if(ENABLE_LOTTIE)
    list(APPEND CDROID_DEPLIBS ${RLOTTIE_LIBRARIES})
endif()

if (litehtml_FOUND)
    list( APPEND CDROID_DEPLIBS litehtml)
    #list(APPEND CDROID_DEPINCLUDES ${LITEHTML_INCLUDE_DIRS})
    #add_definitions(-DENABLE_LITEHTML=1)
endif()

if (PLPLOT_FOUND)
    list( APPEND CDROID_DEPLIBS ${PLPLOT_LIBRARIES})
    list(APPEND CDROID_DEPINCLUDES ${PLPLOT_INCLUDE_DIRS})
    add_definitions(-DENABLE_PLPLOT=1)
endif()

if (ENABLE_BARCODE)
    list( APPEND CDROID_DEPLIBS zint::zint)
endif()

if(ENABLE_FRIBIDI)
    list(APPEND CDROID_DEPINCLUDES ${FRIBIDI_INCLUDE_DIRS})
    list(APPEND CDROID_DEPLIBS ${FRIBIDI_LIBRARIES})
endif(ENABLE_FRIBIDI)

if(ENABLE_MATHGL)
    list(APPEND CDROID_DEPINCLUDES ${MATHGL_INCLUDE_DIRS})
    list(APPEND CDROID_DEPLIBS ${MATHGL_LIBRADIES})
endif()

list(APPEND CDROID_DEPINCLUDES
    ${ZIP_INCLUDE_DIRS}
    ${EXPAT_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
)

message("CDROID_DEPLIBS=${CDROID_DEPLIBS}")

if(ENABLE_PINYIN2HZ)
    list(APPEND OPTIONAL_LIBS pinyin)
    list(APPEND CDROID_DEPLIBS pinyin)
    list(APPEND CDROID_DEPINCLUDES ${CMAKE_SOURCE_DIR}/src/3rdparty/pinyin/include)
endif()

set(SKIP_INSTALL_EXPORT TRUE)

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
