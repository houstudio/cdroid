#
# Check that specified perl modules are installed.
#
# CHECK_PERL_MODULES(VARIABLE PERLMODS)
#
# VARIABLE - variable to store the result
# PERLMODS - list of modules to check
#
macro(CHECK_PERL_MODULES VARIABLE PERLMODS)
    if(NOT DEFINED ${VARIABLE})
      set(${VARIABLE} ON)
      foreach(MOD ${PERLMODS})
	write_file(conftest.pl "use ${MOD};")
	execute_process(
	  COMMAND ${PERL_EXECUTABLE} conftest.pl
	  RESULT_VARIABLE RESULT
	  OUTPUT_QUIET
	  ERROR_QUIET
	  )
	if(NOT RESULT EQUAL 0)
	  set(${VARIABLE} OFF)
	  message(STATUS "WARNING: Perl module ${MOD} not found")
	endif(NOT RESULT EQUAL 0)
      endforeach(MOD ${PERLMODS})
    endif(NOT DEFINED ${VARIABLE})
endmacro(CHECK_PERL_MODULES)
