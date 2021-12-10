
function(CreatePAK ResourceDIR PakPath projectname)
    add_custom_command(TARGET ${projectname} PRE_BUILD
        COMMAND zip -r -0 ${PakPath} ./ 
        WORKING_DIRECTORY ${ResourceDIR})
    message("Package ${ResourceDIR} to:${PakPath}")
endfunction()

function(CreatePO SourceDIR POPath projectname) 
    add_custom_command(TARGET ${projectname} PRE_BUILD
        COMMAND touch ${POPath}
        COMMAND xgettext -d ntvplus -j -c -p${PROJECT_BINARY_DIR} -kTEXT ${SRCS_NTV_PLUS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

