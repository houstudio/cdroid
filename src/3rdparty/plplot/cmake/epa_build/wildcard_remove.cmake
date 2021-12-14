# CMake script to remove files corressponding to a
# FILE_WILDCARD pattern specified (before the -P option) with
# -DFILE_WILDCARD:STRING=<whatever>
file(GLOB file_list ${FILE_WILDCARD})
message(STATUS "file_list to be removed = ${file_list}")
file(REMOVE "${file_list}")
