include .config.in

#
# 定义默认目标
#
all: all_targets

#
# 编译选项
#
CFLAGS = -Os -fPIC -Wall -Werror -Wno-error=stringop-overflow -Wno-error=address -Wno-error=builtin-declaration-mismatch
CFLAGS += -Iinclude/
CFLAGS += -include config.h
CFLAGS += -Wno-pointer-sign
CFLAGS += -Wno-pointer-to-int-cast

obj_dir = .objs/


NO_DMA_COHERENT=0
ENABLE_ARM_L2_CACHE=0
USE_VDK=0
EGL_API_FB=0
STATIC_LINK=0
ABI=0
DEBUG=0
CUSTOM_PIXMAP=0
ANDROID=0
EGL_API_ANDROID=0
USE_3D_VG=0
CONFIG_DOVEXC5_BOARD=0
DIRECTFB_MAJOR_VERSION=1
DIRECTFB_MINOR_VERSION=0
DIRECTFB_MICRO_VERSION=0
include makefile.linux.def

CFLAGS += -Isrc/lib/user -Isrc/lib/os -Isrc/lib/hardware -Isrc/lib/include/


# ----------------------
# 编译 lib2d.so
# ----------------------
module_name = output/lib2d.so

src-$(APP_lib2d) += src/lib/os/gc_hal_user_debug.c
src-$(APP_lib2d) += src/lib/os/gc_hal_user_math.c
src-$(APP_lib2d) += src/lib/os/gc_hal_user_os.c

src-$(APP_lib2d) += src/lib/user/gc_hal_user_brush.c
src-$(APP_lib2d) += src/lib/user/gc_hal_user_brush_cache.c
src-$(APP_lib2d) += src/lib/user/gc_hal_user_dump.c
src-$(APP_lib2d) += src/lib/user/gc_hal_user.c
src-$(APP_lib2d) += src/lib/user/gc_hal_user_raster.c
src-$(APP_lib2d) += src/lib/user/gc_hal_user_heap.c
src-$(APP_lib2d) += src/lib/user/gc_hal_user_query.c
src-$(APP_lib2d) += src/lib/user/gc_hal_user_rect.c
src-$(APP_lib2d) += src/lib/user/gc_hal_user_buffer.c
src-$(APP_lib2d) += src/lib/user/gc_hal_user_surface.c
src-$(APP_lib2d) += src/lib/user/gc_hal_user_queue.c

src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_blt.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_clear.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_context.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_filter_blt_de.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_filter_blt_vr.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_pattern.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_pipe.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_primitive.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_query.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_source.c
src-$(APP_lib2d) += src/lib/hardware/gc_hal_user_hardware_target.c

src-$(APP_lib2d) += src/lib/ingenic2d.c

include tools/build_elf.mk

# ---------------------------
# the end
# ---------------------------
all_targets: $(module_targets)
	@echo "  compiled $(all_modules)"

libs = $(filter %.so,$(all_modules))

ifneq ($(libs),)
define install_libs
	$(Q)cp -f $(libs) $(FS_TARGET_DIR)/usr/lib/
	$(Q)cp -f $(libs) $(FS_STAGING_DIR)/usr/lib/
	@echo "  installed $(libs) include/lib2d/"
endef
define clean_install_libs
	$(Q)rm -f $(addprefix $(FS_TARGET_DIR)/usr/lib/, $(notdir $(libs)))
	$(Q)rm -f $(addprefix $(FS_STAGING_DIR)/usr/lib/, $(notdir $(libs)))
	@echo "  removed $(libs) include/lib2d/"
endef
endif

install:
	$(install_libs)

clean_install:
	$(clean_install_libs)

clean:
	$(Q)rm -rf output/
	$(Q)rm -rf $(module_clean_files)

.PHONY: all clean all_targets clean_install install
