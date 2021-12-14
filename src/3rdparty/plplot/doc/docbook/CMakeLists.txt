if(BUILD_DOC)
  add_subdirectory(bin)
endif(BUILD_DOC)

# Always need to process this directory if only to validate.
add_subdirectory(src)
