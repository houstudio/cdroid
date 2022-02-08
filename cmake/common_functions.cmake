
function(CreatePAK project ResourceDIR PakPath rhpath)
    add_custom_target(${project}_Resource
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/idgen.py ${ResourceDIR} ${rhpath}
        COMMAND zip -r -D -0 ${PakPath} ./
        WORKING_DIRECTORY ${ResourceDIR}
        COMMENT "Pckage Assets from ${ResourceDIR} to:${PakPath}")
    add_dependencies(${project} ${project}_Resource)
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
