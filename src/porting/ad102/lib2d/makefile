
# 检查 .config.in include/config.h 是否有更新
make_sure_config_update:=$(shell tools/check_config.sh $(config_in) 2> /dev/null)

export V
export Q
export CROSS_COMPILE
export FS_TARGET_DIR
export FS_STAGING_DIR

# make V=1 可以看见Makefile运转的细节
ifneq ($(V),1)
Q = @
MAKE_ARG = --no-print-directory
else
Q =
MAKE_ARG =
endif

all:
	$(Q)+make $(MAKE_ARG) -f lib2d.mk all
	$(Q)+make $(MAKE_ARG) -f lib2d_cmd.mk all
clean:
	$(Q)+make $(MAKE_ARG) -f lib2d.mk clean
	$(Q)+make $(MAKE_ARG) -f lib2d_cmd.mk clean

clean_install:
	$(if $(FS_TARGET_DIR),,$(error must set FS_TARGET_DIR))
	$(if $(FS_STAGING_DIR),,$(error must set FS_STAGING_DIR))
	$(Q)+make $(MAKE_ARG) -f lib2d.mk clean_install
	$(Q)+make $(MAKE_ARG) -f lib2d_cmd.mk clean_install

install:
	$(if $(FS_TARGET_DIR),,$(error must set FS_TARGET_DIR))
	$(if $(FS_STAGING_DIR),,$(error must set FS_STAGING_DIR))
	$(Q)+make $(MAKE_ARG) -f lib2d.mk install
	$(Q)+make $(MAKE_ARG) -f lib2d_cmd.mk install

.PHONY: all clean install

