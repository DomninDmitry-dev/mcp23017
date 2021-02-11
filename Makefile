# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	obj-m := ${MODNAME}.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else

#CFLAGS_$(MODNAME_MOD).o := -DDEBUG

.PHONY: default clean

default: ${MODNAME}.c
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURDIR) clean
	@rm -f $(PROGNAME)

endif
