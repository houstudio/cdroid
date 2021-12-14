execute_process(
  COMMAND ${executable} ${argument_list}
  RESULT_VARIABLE return_code
  )

if(NOT return_code EQUAL 0)
  message(FATAL_ERROR "${executable} returned return_code = ${return_code}")
endif(NOT return_code EQUAL 0)
