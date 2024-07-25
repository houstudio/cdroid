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

find_package(Python)

function(CreatePAK project ResourceDIR PakPath rhpath)
    add_custom_target(${project}_assets
        COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/idgen.py ${project} ${ResourceDIR} ${rhpath}
        COMMAND zip -q -r -D -1 ${PakPath} ./  -i "*.xml"
        COMMAND zip -q -r -D -0 ${PakPath} ./  -i "*.png" "*.jpg" "*.jpeg" "*.gif" "*.apng" "*.webp" "*.ttf" "*.otf" "*.ttc"
        WORKING_DIRECTORY ${ResourceDIR}
        COMMENT "Pckage Assets from ${ResourceDIR} to:${PakPath}")
    add_dependencies(${project} ${project}_assets)
    install(FILES ${PakPath} DESTINATION data)
endfunction()

function(CreatePO SourceDIR POPath projectname)
    file(GLOB_RECURSE ${projectname}_POSRCS  "*.c" "*.cc" "*.cpp" "*.h" "*.hpp")
    add_custom_target(${projectname}_po
        COMMAND touch ${POPath}/${projectname}.po
        COMMAND xgettext -d ${projectname} -j -c -p${POPath} -kTEXT ${${projectname}_POSRCS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_dependencies( ${projectname} ${projectname}_po)
endfunction()

function(Translate pofile transtopath)
   add_custom_target(translate
      #po2json translate   pofile to string_xx.json
      COMMAND ${Python_EXECUTABLE}  ${CMAKE_SOURCE_DIR}/scripts/po2json.py ${pofile}
      # convert xls (after your custom finished translate) to string_xx.json for pak 
      #COMMAND python  ${CMAKE_SOURCE_DIR}/src/tools/po2json.py ${CMAKE_CURRENT_BINARY_DIR}/newglee.po.xls
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
        COMMAND git describe --tags --abbrev=0
        OUTPUT_VARIABLE CDVERSION
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

    if(NOT CDVERSION)
        set(CDVERSION "2.0.0")
        set(COMMIT_COUNT "0");
        set(LAST_COMMIT_HASH "00000000")
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
        set(${TARGET}_COMMITID ${LAST_COMMIT_HASH} PARENT_SCOPE)
    endif()
endfunction()

