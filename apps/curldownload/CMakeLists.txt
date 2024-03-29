project(curldownload C CXX)

set(CMAKE_CXX_STANDARD 11)
find_package(CURL)
include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/cairo
   ${CMAKE_BINARY_DIR}/include/epg
   ${CMAKE_BINARY_DIR}/include/freetype2
)
aux_source_directory(./ CURLDLD_SRCS)
add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)
link_directories(${CMAKE_BINARY_DIR}/lib)
find_package(OpenSSL)
find_package(MBEDTLS)

if(CURL_FOUND AND OPENSSL_FOUND AND MBEDTLS_FOUND)
  add_executable(curldownload ${CURLDLD_SRCS})
  target_link_libraries(curldownload PRIVATE atomic cdroid -Wl,--start-group
	  ${MBEDTLS_LIBRARIES}
	  ${OPENSSL_LIBRARIES}
	  ${CURL_LIBRARIES}
	  -Wl,--end-group)
  install(TARGETS curldownload DESTINATION bin)
endif()

