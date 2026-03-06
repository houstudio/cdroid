project(ingenic2d C CXX)
set(G2DSRCS
  src/lib/os/gc_hal_user_debug.c
  src/lib/os/gc_hal_user_math.c
  src/lib/os/gc_hal_user_os.c

  src/lib/user/gc_hal_user_brush.c
  src/lib/user/gc_hal_user_brush_cache.c
  src/lib/user/gc_hal_user_dump.c
  src/lib/user/gc_hal_user.c
  src/lib/user/gc_hal_user_raster.c
  src/lib/user/gc_hal_user_heap.c
  src/lib/user/gc_hal_user_query.c
  src/lib/user/gc_hal_user_rect.c
  src/lib/user/gc_hal_user_buffer.c
  src/lib/user/gc_hal_user_surface.c
  src/lib/user/gc_hal_user_queue.c

  src/lib/hardware/gc_hal_user_hardware_blt.c
  src/lib/hardware/gc_hal_user_hardware_clear.c
  src/lib/hardware/gc_hal_user_hardware_context.c
  src/lib/hardware/gc_hal_user_hardware_filter_blt_de.c
  src/lib/hardware/gc_hal_user_hardware_filter_blt_vr.c
  src/lib/hardware/gc_hal_user_hardware.c
  src/lib/hardware/gc_hal_user_hardware_pattern.c
  src/lib/hardware/gc_hal_user_hardware_pipe.c
  src/lib/hardware/gc_hal_user_hardware_primitive.c
  src/lib/hardware/gc_hal_user_hardware_query.c
  src/lib/hardware/gc_hal_user_hardware_source.c
  src/lib/hardware/gc_hal_user_hardware_target.c

  src/lib/ingenic2d.c
  )
 
include_directories(
	include/
	include/lib2d
	src/lib/hardware
	src/lib/include
	src/lib/os
	src/lib/user)
set(INGENIC_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/lib2d PARENT_SCOPE)
add_library(ingenic2d STATIC ${G2DSRCS})
link_directories(./)
#target_link_libraries(ingenic2d hardware2 m)
