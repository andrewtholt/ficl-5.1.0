FICL_VERSION=5.1.0
FICL_SITE=/home/andrewh/Source/ficl-5.1.0/
FICL_SITE_METHOD=local
FICL_INSTALL_TARGET=YES

define FICL_CLEAN_CMDS
	$(MAKE) -C $(@D) clean
endef

define FICL_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

$(eval $(generic-package))

