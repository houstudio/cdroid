# CMake does not automatically add --whole-archive when building shared objects from
# a list of convenience libraries. This can lead to missing symbols in the final output.
# We add --whole-archive to all libraries manually to prevent the linker from trimming
# symbols that we actually need later.
macro(ADD_WHOLE_ARCHIVE_TO_LIBRARIES _list_name)
    foreach (library IN LISTS ${_list_name})
      list(APPEND ${_list_name}_TMP -Wl,--whole-archive ${library} -Wl,--no-whole-archive)
    endforeach ()
    set(${_list_name} "${${_list_name}_TMP}")
endmacro()

MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child} AND
            EXISTS ${curdir}/${child}/CMakeLists.txt)
                LIST(APPEND dirlist ${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()

if(NOT Python_EXECUTABLE)
    #compiled error with vcpkg (for Android platform)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    # find_package(Python3) sets Python3_EXECUTABLE, not Python_EXECUTABLE — wire it up
    # so the lxml/Pillow checks and the packaging scripts are invoked with a real interpreter
    # (otherwise ${Python_EXECUTABLE} expands empty and every script silently falls back).
    set(Python_EXECUTABLE ${Python3_EXECUTABLE})
endif()

function(CreatePAK project ResourceDIR PakPath rhpath)
    # One Python class (scripts/pakbuilder.py) does it all: generates R.h/ID.xml
    # (reusing idgen.py) and builds the pak — stripped XML + aapt-compiled 9-patch
    # (cdNp chunk, borderless) + verbatim binaries. lxml+Pillow are hard deps;
    # pakbuilder.py fails loud if either is missing (no silent half-broken path).
    #
    # Binary AXML is the only mode: pakbuilder gets the extra aapt2 args and
    # produces binary AXML layouts + resources.arsc (no ENABLE_BINARY_XML gate
    # anymore — the text-XML path is retired).
    set(extra_args "")
    set(_framework_apk "${CMAKE_BINARY_DIR}/framework.apk")
    if(EXISTS "${CDROID_SDK_RES}")
        if("${project}" STREQUAL "cdroid")
            # Only cdroid.pak gets the full SDK framework res. The built framework.apk
            # is also persisted (--framework-apk-out) so app paks can -I CDROID's own
            # framework: package 'android', real public ids, plus the 0x010d CDROID
            # extension attrs (pattern/frameDuration/wheelItemCount/...) that a stock
            # android.jar doesn't expose (private or absent).
            set(extra_args "${CDROID_AAPT2}" "${CDROID_ANDROID_JAR}" "${CDROID_SDK_RES}"
                           "--framework-apk-out" "${_framework_apk}")
            if(CDROID_SDK_RES_FILTER AND EXISTS "${CDROID_SDK_RES_FILTER}")
                list(APPEND extra_args "${CDROID_SDK_RES_FILTER}")
            endif()
            # Global overlay (all apps of this build): shadow the shared cdroid.pak
            # values, e.g. per-chipset touch/fling tuning via the <product>.txt
            # options file.
            if(CDROID_OVERLAY AND EXISTS "${CDROID_OVERLAY}")
                list(APPEND extra_args "--overlay" "${CDROID_OVERLAY}")
                message(STATUS "CreatePAK(${project}): global overlay ${CDROID_OVERLAY}")
            endif()
            message(STATUS "CreatePAK(${project}): SDK framework res mode")
        else()
            # App paks: compile their own XML via aapt2 (binary AXML), no SDK res.
            # Framework -I = CDROID's own framework.apk (NOT android.jar): it carries
            # the same public attr table the runtime arsc was built from, including
            # the 0x010d extensions, so compiled attr ids match the runtime exactly.
            # --widgetex-apk lets app aapt2 link -I the fixed-id 0x02 shared lib so
            # widgetEx attrs resolve to 0x02 (matching the runtime styleable) instead
            # of being re-declared at 0x7f in the app's own arsc.
            set(extra_args "${CDROID_AAPT2}" "${_framework_apk}"
                           "--widgetex-apk" "${CMAKE_BINARY_DIR}/widgetex.apk")
            message(STATUS "CreatePAK(${project}): app binary AXML mode (own res only)")
        endif()
    endif()
    add_custom_target(${project}_assets
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/pakbuilder.py
                ${project} ${ResourceDIR} ${PakPath} ${rhpath} ${extra_args}
        COMMAND cp ${PakPath} ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${ResourceDIR}
        COMMENT "Package Assets from ${ResourceDIR} to:${PakPath}")
    add_dependencies(${project} ${project}_assets)
    # App paks need widgetex.apk (for -I linking); ensure widgetex builds first.
    if(TARGET widgetex_assets AND NOT "${project}" STREQUAL "widgetex" AND NOT "${project}" STREQUAL "cdroid")
        add_dependencies(${project}_assets widgetex_assets)
    endif()
    # App paks -I framework.apk, which the cdroid SDK pak produces — build it first.
    if(TARGET cdroid_assets AND NOT "${project}" STREQUAL "cdroid")
        add_dependencies(${project}_assets cdroid_assets)
    endif()
    # Per-app framework overlay: apps/<name>/overlay/ shadows the framework res
    # with the same --overlay semantics (values* entries merge by (type,name),
    # everything else whole-file). Builds a dedicated cdroid.pak into the app's
    # binary dir — App::findSharedPak probes the binary's own directory first,
    # so the overlaid pak applies to THIS app only; other apps/samples keep the
    # shared out-dir-root cdroid.pak. Details worth keeping straight:
    #  - namespace "cdroid" reuses the SDK -x pipeline (scrubs + pinned ids);
    #    its work dir <app>/cdroid_pakbuild cannot collide with the shared build.
    #  - rh_path points into that work dir: R.h/internal_R.h regenerate there and
    #    never race the shared build's source-tree headers (ids are identical —
    #    overlay only changes values of pinned names).
    #  - NO --framework-apk-out: the shared framework.apk that app paks -I stays
    #    the base one; ids match, only default values differ.
    #  - No cp to the binary root and no install: must not shadow the shared pak.
    #  - If a global CDROID_OVERLAY is set it applies first, the app overlay wins.
    if(NOT "${project}" STREQUAL "cdroid" AND NOT "${project}" STREQUAL "widgetex"
       AND EXISTS "${CDROID_SDK_RES}" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/overlay")
        set(_ov_extra "")
        if(CDROID_SDK_RES_FILTER AND EXISTS "${CDROID_SDK_RES_FILTER}")
            set(_ov_extra "${CDROID_SDK_RES_FILTER}")
        endif()
        set(_ov_flags "")
        if(CDROID_OVERLAY AND EXISTS "${CDROID_OVERLAY}")
            list(APPEND _ov_flags "--overlay" "${CDROID_OVERLAY}")
        endif()
        list(APPEND _ov_flags "--overlay" "${CMAKE_CURRENT_SOURCE_DIR}/overlay")
        add_custom_target(${project}_framework_assets
            COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/pakbuilder.py cdroid
                    ${CDROID_SDK_RES} ${CMAKE_CURRENT_BINARY_DIR}/cdroid.pak
                    ${CMAKE_CURRENT_BINARY_DIR}/cdroid_pakbuild/R.h
                    ${CDROID_AAPT2} ${CDROID_ANDROID_JAR} ${CDROID_SDK_RES} ${_ov_extra}
                    ${_ov_flags}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Framework overlay pak for ${project} (${CMAKE_CURRENT_SOURCE_DIR}/overlay)")
        add_custom_command(TARGET ${project} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_CURRENT_BINARY_DIR}/cdroid.pak
                    $<TARGET_FILE_DIR:${project}>/cdroid.pak
            COMMENT "Deploy overlaid cdroid.pak beside ${project} binary")
        add_dependencies(${project} ${project}_framework_assets)
        message(STATUS "CreatePAK(${project}): framework overlay from ${CMAKE_CURRENT_SOURCE_DIR}/overlay")
    endif()
    install(FILES ${PakPath} DESTINATION data)
endfunction()

function(Translate pofile transtopath)
   add_custom_target(translate
      #po2json translate   pofile to string_xx.json
      COMMAND ${Python_EXECUTABLE}  ${CMAKE_SOURCE_DIR}/scripts/po2json.py ${pofile}
      # convert xls (after your custom finished translate) to string_xx.json for pak 
      #COMMAND python  ${CMAKE_SOURCE_DIR}/scripts/po2json.py ${CMAKE_CURRENT_BINARY_DIR}/newglee.po.xls
      COMMAND cp  ${PROJECT_BINARY_DIR}/string*.json ${PROJECT_SOURCE_DIR}/assets/strings
      BYPRODUCTS ntvplus
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      COMMENT "Translate strings resource...${PROJECT_BINARY_DIR}"
   )
endfunction()

function(GetGitVersion TARGET)
    execute_process(
        COMMAND git describe --tags
        OUTPUT_VARIABLE LATEST_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND git rev-parse --short ${LATEST_TAG}
        OUTPUT_VARIABLE LAST_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND git rev-list --count ${LATEST_TAG}
        #We can change LATEST_TAG to CDVERSION to get commit count since your TAG is created"
        OUTPUT_VARIABLE COMMIT_COUNT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(CHANGELOG_FILE "${CMAKE_SOURCE_DIR}/ChangeLog.md")
    file(READ "${CHANGELOG_FILE}" CHANGELOG_CONTENT)
    set(VERSION_REGEX "([0-9]+)\\.([0-9]+)\\.([0-9]+)")
    string(REGEX MATCH "${VERSION_REGEX}" FULL_MATCH "${CHANGELOG_CONTENT}")
    if(FULL_MATCH)
        set(CDVERSION "${FULL_MATCH}")
    else()
        execute_process(
            COMMAND git describe --tags --abbrev=0
            OUTPUT_VARIABLE CDVERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()

    # Extract MAJOR, MINOR, PATCH from VERSION if MAJOR, MINOR, PATCH are provided
    if(DEFINED CDVERSION)
        string(REGEX MATCHALL "([0-9]+)\\.([0-9]+)(\\.[0-9]+)?" matches ${CDVERSION})
        set(${TARGET}_VERSION_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
        set(${TARGET}_VERSION_MINOR ${CMAKE_MATCH_2} PARENT_SCOPE)
        #message(FATAL_ERROR "${TARGET}_MAJOR=${${TARGET}_MAJOR}=${CMAKE_MATCH_1}")
        if(CMAKE_MATCH_3)
            string(REPLACE "." "" CMAKE_MATCH_3 ${CMAKE_MATCH_3})
            set(${TARGET}_VERSION_PATCH ${CMAKE_MATCH_3} PARENT_SCOPE)
            set(${TARGET}_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}" PARENT_SCOPE)
        else()
            set(${TARGET}_PATCH "0" PARENT_SCOPE)
            set(${TARGET}_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.0" PARENT_SCOPE)
        endif()
    endif()
    if(DEFINED COMMIT_COUNT AND DEFINED LAST_COMMIT_HASH)
        set(${TARGET}_BUILD_NUMBER ${COMMIT_COUNT}  PARENT_SCOPE)
        string(TOUPPER ${LAST_COMMIT_HASH} LAST_COMMIT_HASH)
        set(${TARGET}_COMMITID ${LAST_COMMIT_HASH} PARENT_SCOPE)
    endif()
endfunction()

