#
# 定义默认目标
#
include .config.in
all: all_targets

#
# 编译选项
#
CFLAGS = -Os
CFLAGS += -Iinclude/
CFLAGS += -include config.h -Wall -Werror
CFLAGS += -Wno-pointer-sign
CFLAGS += -Wno-pointer-to-int-cast

LDFLAGS = -Loutput/ -lhardware2 -lpthread -l2d

obj_dir = .objs/

check_lib = $(shell tools/check_lib.sh $(CROSS_COMPILE)gcc $1)

module_name = output/ingenic_2d_scale
src-$(APP_ingenic2d_cmd) += src/cmd/main_scale.c
include tools/build_elf.mk

module_name = output/ingenic_2d_rotater
src-$(APP_ingenic2d_cmd) += src/cmd/main_rotater.c
include tools/build_elf.mk

module_name = output/ingenic_2d_fill_rect
src-$(APP_ingenic2d_cmd) += src/cmd/main_fill_rect.c
include tools/build_elf.mk

module_name = output/ingenic_2d_blend
src-$(APP_ingenic2d_cmd) += src/cmd/main_blend.c
include tools/build_elf.mk

module_name = output/ingenic_2d_convert
src-$(APP_ingenic2d_cmd) += src/cmd/main_convert.c
include tools/build_elf.mk

module_name = output/ingenic_2d_draw_lines
src-$(APP_ingenic2d_cmd) += src/cmd/main_draw_lines.c
include tools/build_elf.mk

module_name = output/ingenic_2d_filp
src-$(APP_ingenic2d_cmd) += src/cmd/main_filp.c
include tools/build_elf.mk

# ---------------------------
# the end
# ---------------------------
all_targets: $(module_targets)
	@echo "  compiled $(all_modules)"

cmds = $(filter-out %.so,$(all_modules))

ifneq ($(cmds),)
define install_cmds
	$(Q)cp -f $(cmds) $(FS_TARGET_DIR)/usr/bin/
	@echo "  installed $(cmds)"
endef
define clean_install_cmds
	$(Q)rm -f $(addprefix $(FS_TARGET_DIR)/usr/bin/, $(notdir $(cmds)))
	@echo "  removed $(cmds)"
endef
endif

install:
	$(install_cmds)

clean_install:
	$(clean_install_cmds)

clean:
	$(Q)rm -rf output/
	$(Q)rm -rf $(module_clean_files)

.PHONY: all clean all_targets clean_install install
