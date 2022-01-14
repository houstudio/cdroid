
function(CreatePAK ResourceDIR PakPath projectname)
    add_custom_target(${projectname}_res
        COMMAND zip -r  -0 ${PakPath} ./ 
        WORKING_DIRECTORY ${ResourceDIR})
    add_dependencies(${projectname} ${projectname}_res)
    message("Package ${ResourceDIR} to:${PakPath}")
    install(FILES ${PakPath} DESTINATION data)
endfunction()

function(CreatePO SourceDIR POPath projectname) 
    add_custom_target(${projectname}_po
        COMMAND touch ${POPath}
        COMMAND xgettext -d ntvplus -j -c -p${PROJECT_BINARY_DIR} -kTEXT ${SRCS_NTV_PLUS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_dependencies( ${projectname} ${projectname}_po)
endfunction()

function(Translate pofile transtopath)
   add_custom_target(translate
      COMMAND python  ${CMAKE_SOURCE_DIR}/src/tools/po2json.py ${pofile}
      COMMAND cp  ${PROJECT_BINARY_DIR}/string*.json ${PROJECT_SOURCE_DIR}/assets/strings
      BYPRODUCTS ntvplus
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      COMMENT "Translate strings resource...${PROJECT_BINARY_DIR}"
   )
endfunction()
