
function(CreatePAK ResourceDIR PakPath projectname)
    add_custom_target(${projectname}_res
        COMMAND zip -r  -0 ${PakPath} ./ 
        DEPENDS ${projectname}
        WORKING_DIRECTORY ${ResourceDIR})
    message("Package ${ResourceDIR} to:${PakPath}")
endfunction()

function(CreatePO SourceDIR POPath projectname) 
    add_custom_target(${projectname}_po
        COMMAND touch ${POPath}
        COMMAND xgettext -d ntvplus -j -c -p${PROJECT_BINARY_DIR} -kTEXT ${SRCS_NTV_PLUS}
        DEPENDS ${projectname}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

