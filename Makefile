KERNELRELEASE ?= $(shell uname -r)
KDIR ?= /lib/modules/$(KERNELRELEASE)/build

obj-m := fake_dp_dual_mode.o
fake_dp_dual_mode-y := module.o

all: modules ;

%:
	$(MAKE) -C "$(KDIR)" "M=$(CURDIR)" $@
