# The CMake logic fragment hould be run with cmake -P with URL, FILE, and
# MD5 variables defined by -D options (before the -P option pointing to
# this file).

message(STATUS "Download from ${URL} to ${FILE} and check downloaded result has an md5sum of ${MD5}")

file(DOWNLOAD ${URL} ${FILE} EXPECTED_MD5 ${MD5})