project (ali3528 C CXX)

set(ALI_SRCS 
    ngl_os.c
    ngl_msgq.c
    ngl_timer.c
    ngl_nvm.c
    graph_ali.c
    ngl_tuner.c
    ngl_panel.c
    ngl_pvr.c
    ngl_ir.cc
    ngl_mediaplayer.c
    )

if (ENABLE_DTV)
    list(APPEND ALI_SRCS  ngl_dsc.c ngl_smc.c  ngl_tuner.c ngl_dmx.cc 
          ngl_nvm.c  ngl_pvr.c ngl_snd.c ngl_video.c  alidisp.c  )
endif()
list(APPEND ALI_LIBS pthread  direct directfb aui)


link_directories(
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/usr/lib
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/lib
)

include_directories(./
    ../include
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/deps/include
    ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/usr/include/directfb
    ${TOOLCHAIN_DIR}/host/usr/mipsel-buildroot-linux-gnu/sysroot/usr/include/directfb
)

add_library(ali3528 STATIC ${ALI_SRCS} )
target_link_libraries(ali3528 ${ALI_LIBS})

