
function(CreatePAK ResourceDIR PakPath projectname)
    add_custom_target(${projectname}_res
        COMMAND zip -r  -0 ${PakPath} ./ 
        WORKING_DIRECTORY ${ResourceDIR})
    add_dependencies(${projectname} ${projectname}_res)
    message("Package ${ResourceDIR} to:${PakPath}")
endfunction()

function(CreatePO SourceDIR POPath projectname) 
    add_custom_target(${projectname}_po
        COMMAND touch ${POPath}
        COMMAND xgettext -d ntvplus -j -c -p${PROJECT_BINARY_DIR} -kTEXT ${SRCS_NTV_PLUS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_dependencies( ${projectname} ${projectname}_po)
endfunction()

