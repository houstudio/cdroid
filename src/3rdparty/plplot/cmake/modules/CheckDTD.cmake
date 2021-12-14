macro(CheckDTD VARIABLE title docbookb_ss_dtd style_spec_use
	external_specification docbook_dtd jade_output_type
	origin_package)
  if(NOT DEFINED ${VARIABLE})
    set(CONFTEST "testdtd")
    file(WRITE ${CMAKE_BINARY_DIR}/${CONFTEST}.dsl
    "<!DOCTYPE style-sheet PUBLIC \"${DSSSL_DTD_PUBID}\" ${docbookb_ss_dtd}>
    <style-sheet>
    <style-specification ${style_spec_use}>
    <style-specification-body>
    (define %root-filename% \"${CONFTEST}\")
    </style-specification-body>
    </style-specification>
    ${external_specification}
    </style-sheet>"
    )
    file(WRITE ${CMAKE_BINARY_DIR}/${CONFTEST}.xml
    "<?xml version=\"1.0\"?>
    <!DOCTYPE book ${docbook_dtd}>
    <book> </book>"
    )
    execute_process(
    COMMAND ${OPENJADE} ${SGML_CATALOGS}
    -d ${CMAKE_BINARY_DIR}/${CONFTEST}.dsl
    -t ${jade_output_type}
    -o ${CMAKE_BINARY_DIR}/${CONFTEST}.out
    ${XML_DECL} ${CMAKE_BINARY_DIR}/${CONFTEST}.xml
    OUTPUT_FILE "${JADELOG}.x"
    ERROR_FILE "${JADELOG}.x"
    )
    file(READ "${CMAKE_BINARY_DIR}/${JADELOG}.x" RESULT)
    if("${RESULT}" MATCHES ":E:")
      set(${VARIABLE} OFF CACHE BOOL "Checking for ${title}" FORCE)
      message(STATUS "WARNING: ${title} not found")
    else("${RESULT}" MATCHES ":E:")
      set(${VARIABLE} ON CACHE BOOL "Checking for ${title}" FORCE)
      message(STATUS "${title} found")
    endif("${RESULT}" MATCHES ":E:")
  endif(NOT DEFINED ${VARIABLE})
endmacro(CheckDTD)
