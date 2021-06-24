file( GLOB conf_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.conf" )
install( FILES README ${conf_files} DESTINATION etc/fonts/conf.d )
install( FILES README ${conf_files} DESTINATION etc/fonts/conf.avail )