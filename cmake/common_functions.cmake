
function(CreatePAK ResourceDIR PakPath)
    execute_process(COMMAND zip -r -0 ${PakPath} ./ -i *
        WORKING_DIRECTORY ${ResourceDIR})
    message("Package ${ResourceDIR} to:${PakPath}")
endfunction()

function(CreatePO SourceDIR POPath) 
    execute_process(
        COMMAND touch ${POPath}
        COMMAND xgettext -d ntvplus -j -c -p${PROJECT_BINARY_DIR} -kTEXT ${SRCS_NTV_PLUS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

