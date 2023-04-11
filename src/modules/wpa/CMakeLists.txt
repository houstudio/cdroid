project(wpaclient C CXX)

set( WPA_SRCS wpa_ctrl.c wpasockclient.cc)
add_definitions(-DCONFIG_CTRL_IFACE_UNIX=1 -DCONFIG_CTRL_IFACE=1)
include_directories(./ ../../gui
	../../gui/cairomm
	${CMAKE_BINARY_DIR}/include
	${CDROID_DEPINCLUDES}
	${CMAKE_BINARY_DIR}/include/gui
	${CMAKE_BINARY_DIR}/include/porting)

add_library(wpaclient ${WPA_SRCS})
add_executable(wpatest main.cc)
target_link_libraries(wpatest wpaclient cdroid)

