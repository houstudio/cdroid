#set platform include dirs & library path

set(HAL_EXTRA_INCDIRS
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/usr/include/directfb 
 ${TOOLCHAIN_DIR}/host/usr/mipsel-buildroot-linux-gnu/sysroot/usr/include/directfb
)
set(HAL_EXTRA_LIBDIRS
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/usr/lib
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/lib
)

set(HAL_EXTRA_DLIBS pthread aui direct directfb)
set(HAL_EXTRA_SLIBS pthread aui direct directfb)

set(SOURCES_ali3538
   ngl_os.c
   ngl_msgq.c
   ngl_dmx.cc
   ngl_timer.c
   ngl_nvm.c
   ngl_tuner.c
   ngl_smc.c
   ngl_video.c
   ngl_snd.c
   ngl_disp.c
   ngl_panel.c
   ngl_pvr.c
   ngl_ir.cc
   ngl_mediaplayer.c
   ngl_dsc.c
)
