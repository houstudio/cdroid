
function(CreatePAK project ResourceDIR PakPath rhpath)
    add_custom_target(${project}_assets
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/idgen.py ${ResourceDIR} ${rhpath}
        COMMAND zip -r -D -0 ${PakPath} ./
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
      COMMAND python  ${CMAKE_SOURCE_DIR}/scripts/po2json.py ${pofile} 
      # convert xls (after your custom finished translate) to string_xx.json for pak 
      #COMMAND python  ${CMAKE_SOURCE_DIR}/src/tools/po2json.py ${CMAKE_CURRENT_BINARY_DIR}/newglee.po.xls
      COMMAND cp  ${PROJECT_BINARY_DIR}/string*.json ${PROJECT_SOURCE_DIR}/assets/strings
      BYPRODUCTS ntvplus
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      COMMENT "Translate strings resource...${PROJECT_BINARY_DIR}"
   )
endfunction()

option(JSONCPP_WITH_TESTS "Compile and (for jsoncpp_check) run JsonCpp test executables" OFF)
option(JSONCPP_WITH_POST_BUILD_UNITTEST "Automatically run unit-tests as a post build step" OFF)


MACRO(BUILD_CMAKE_PACKAGE _pkg_name)
    SET(PACKAGE_BINARY_PATH ${CMAKE_BINARY_DIR}/src/3rdparty/${_pkg_name})
    ADD_CUSTOM_COMMAND(
        TARGET ${_pkg_name}_3rd
        COMMAND make -j8
        COMMAND make install
	WORKING_DIRECTORY ${PACKAGE_BINARY_PATH}
	COMMENT ".......building ${_pkg_name} at ${PACKAGE_BINARY_PATH}"
        )
ENDMACRO(BUILD_CMAKE_PACKAGE)

MACRO(ADD_CMAKE_PACKAGE _pkg_name)
    add_subdirectory(${_pkg_name})
    SET(THIRDPARTY_BUILD_TARGETS ${THIRDPARTY_BUILD_TARGETS} ${_pkg_name}_3rd)
    ADD_CUSTOM_TARGET(${_pkg_name}_3rd)
    BUILD_CMAKE_PACKAGE(${_pkg_name})
ENDMACRO(ADD_CMAKE_PACKAGE)

MACRO(BUILD_3RD)
   execute_process(
	COMMAND make cdroiddeps
	COMMENT ".......building cdroiddeps $CMAKE_BINARY_DIR}"
	WORKING_DIRECTORY $CMAKE_BINARY_DIR}/src/3rdparty 
	    ) 
ENDMACRO(BUILD_3RD)
