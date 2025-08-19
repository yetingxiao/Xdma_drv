SHELL = /bin/bash
#
# optional makefile parameters:
# - DEBUG=<0|1>,	enable verbose debug print-out in the driver
DEBUG	=	0
# - config_bar_num=,	xdma pci config bar number
# - xvc_bar_num=,	xvc pci bar #
# - xvc_bar_offset=,	xvc register base offset
# 
# BASESUBDIRS:= uartns550_v3_6
# SUBDIRS:= $(BASESUBDIRS) 

# UART_SRC_DIR := .	 
# #UART_SRC = $(UART_SRC_DIR)/$(wildcard *.c)
# UART_SRC =  $(UART_SRC_DIR)/xil_assert.c \
			# $(UART_SRC_DIR)/xuartns550.c \
			# $(UART_SRC_DIR)/xuartns550_format.c \
			# $(UART_SRC_DIR)/xuartns550_g.c \
			# $(UART_SRC_DIR)/xuartns550_intr.c \
			# $(UART_SRC_DIR)/xuartns550_l.c \
			# $(UART_SRC_DIR)/xuartns550_options.c \
			# $(UART_SRC_DIR)/xuartns550_selftest.c \
			# $(UART_SRC_DIR)/xuartns550_sinit.c \
			# $(UART_SRC_DIR)/xuartns550_stats.c \
			# $(UART_SRC_DIR)/xil_printf.c	\
			# $(UART_SRC_DIR)/xil_io.c
# #UART_OBJS = $(UART_SRCS:.c=.o)
# UART_OBJS = $(UART_SRC_DIR)/xil_assert.o \
			# $(UART_SRC_DIR)/xuartns550.o \
			# $(UART_SRC_DIR)/xuartns550_format.o \
			# $(UART_SRC_DIR)/xuartns550_g.o \
			# $(UART_SRC_DIR)/xuartns550_intr.o \
			# $(UART_SRC_DIR)/xuartns550_l.o \
			# $(UART_SRC_DIR)/xuartns550_options.o \
			# $(UART_SRC_DIR)/xuartns550_selftest.o \
			# $(UART_SRC_DIR)/xuartns550_sinit.o \
			# $(UART_SRC_DIR)/xuartns550_stats.o	\
			# $(UART_SRC_DIR)/xil_printf.o	\
			# $(UART_SRC_DIR)/xil_io.o
            # # ... 其他 .o 文件
 
#UART_SRC = $(UART_SRC_DIR)/$(wildcard *.c)
UART_SRC =  xil_assert.c \
			xuartns550.c \
			xuartns550_format.c \
			xuartns550_g.c \
			xuartns550_intr.c \
			xuartns550_l.c \
			xuartns550_options.c \
			xuartns550_selftest.c \
			xuartns550_sinit.c \
			xuartns550_stats.c \
			xil_printf.c	\
			xil_io.c		\
			xuart_cdev.c	\
			xparameters.c
#UART_OBJS = $(UART_SRCS:.c=.o)
UART_OBJS = xil_assert.o \
			xuartns550.o \
			xuartns550_format.o \
			xuartns550_g.o \
			xuartns550_intr.o \
			xuartns550_l.o \
			xuartns550_options.o \
			xuartns550_selftest.o \
			xuartns550_sinit.o \
			xuartns550_stats.o	\
			xil_printf.o	\
			xil_io.o		\
			xuart_cdev.o	\
			xparameters.o
            # ... 其他 .o 文件

TARGET_MODULE:=xdma
############## remove by ycf
#TARGET_MODULE:=vi53xx
####################### add by ycf
####################### add by ycf
#libxdmaVi53xx.c vi53xx_cndev_core.c 
obj-ko := $(TARGET_MODULE).ko
obj-c := libxdma.c xdma_cdev.c cdev_ctrl.c cdev_events.c cdev_sgdma.c cdev_xvc.c cdev_bypass.c xdma_mod.c xdma_thread.c board_info.c vi53xx_proc.c vi53xx_xdma_ctrl.c  $(UART_SRC)
obj-tar :=  xdma_module_src.tgz
INSTALL_SCRIPT = xdma_install
########################## add by ycf

ifneq ($(xvc_bar_num),)
	XVC_FLAGS += -D__XVC_BAR_NUM__=$(xvc_bar_num)
endif

ifneq ($(xvc_bar_offset),)
	XVC_FLAGS += -D__XVC_BAR_OFFSET__=$(xvc_bar_offset)
endif

$(warning XVC_FLAGS: $(XVC_FLAGS).)

topdir := $(shell cd $(src)/.. && pwd)



EXTRA_CFLAGS := -I$(topdir)/include $(XVC_FLAGS)
ifeq ($(DEBUG),1)
	EXTRA_CFLAGS += -D__LIBXDMA_DEBUG__
endif
ifneq ($(config_bar_num),)
	EXTRA_CFLAGS += -DXDMA_CONFIG_BAR_NUM=$(config_bar_num)
endif
#EXTRA_CFLAGS += -DINTERNAL_TESTING
########################## add by ycf
#EXTRA_INCLUDEDIR := $(UART_SRC_DIR)/include
#EXTRA_INCLUDEDIR := ./uartns550_v3_6/include
#EXTRA_INCLUDEDIR := ./include
INCLUDES :=-I/usr/include/
#INCLUDES :=-I/usr/src/linux-headers-4.9.178-rt131-1/include/linux/
EXTRA_CFLAGS +=   $(CC_ARCH)  -pthread 
EXTRA_CFLAGS += -I$(EXTRA_INCLUDEDIR) 
EXTRA_CFLAGS += $(INCLUDES)
#EXTRA_CFLAGS += -H

#EXTRA_CFLAGS += -ggdb3 -fPIC -O0 -Wall  -Wpointer-arith 
########################## add by ycf
#libxdmaVi53xx.o vi53xx_cndev_core.o
ifneq ($(KERNELRELEASE),)
	$(TARGET_MODULE)-objs := libxdma.o xdma_cdev.o cdev_ctrl.o cdev_events.o cdev_sgdma.o cdev_xvc.o cdev_bypass.o xdma_mod.o xdma_thread.o board_info.o vi53xx_proc.o vi53xx_xdma_ctrl.o   $(UART_OBJS) 
	obj-m := $(TARGET_MODULE).o 
else
	BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
	PWD:=$(shell pwd)
	
include ../../../defines.mk	

# 确保所有的.c文件都被编译
#$(SUBDIRS): 
#	$(MAKE) -C $@ $(MAKECMDGOALS)
#$(SUBDIRS)
all :tar 
	#$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules
	#$(MAKE) -C $(UART_SRC_DIR) modules
	$(MAKE) -C  $(LINUX_DIR) M=$(PWD) modules
	
clean:
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean
	@/bin/rm -f *.ko modules.order *.mod.c *.o *.o.ur-safe .*.o.cmd
	####################### add by ycf
	@/bin/rm -f Module.symvers $(obj-tar)
	####################### add by ycf

install:all
	############## remove by ycf
	# @rm -f /lib/modules/5.15.0-67-generic/extra/xdma.ko
	# @echo "installing kernel modules to /lib/modules/$(shell uname -r)/xdma ..."
	# @mkdir -p -m 755 /lib/modules/$(shell uname -r)/xdma
	# @install -v -m 644 *.ko /lib/modules/$(shell uname -r)/xdma
	# @depmod -a || true
	#################### remove by ycf
	
	####################### add by ycf
	$(INSTALL) -d $(ETAS_BASE_DIR)/modules
	$(INSTALL) -m 644 $(obj-ko) $(obj-tar) $(ETAS_BASE_DIR)/modules
#	$(INSTALL) -d $(ETAS_BASE_DIR)/etc/siminit
#	S(INSTALL) vi53xx-siminit $(ETAS_BASE_DIR)/etc/siminit/50-vi53xx
#	S (INSTALL) -d  $(ETAS_BASE_DIR)/etc/model.d
#	$(INSTALL) vi53xx-modelinit  $(ETAS_BASE_DIR)/etc/model.d
	$(INSTALL) -d  $(DESTDIR)/etc/init.d/
	$(INSTALL) $(INSTALL_SCRIPT)  $(DESTDIR)/etc/init.d/
	####################### add by ycf

uninstall:
	############## remove by ycf
	# @echo "Un-installing /lib/modules/$(shell uname -r)/xdma ..."
	# @/bin/rm -rf /lib/modules/$(shell uname -r)/xdma
	# @depmod -a
	############## remove by ycf
	
	####################### add by ycf
	@echo "Un-installing $(ETAS_BASE_DIR)/modules/$(obi-ko) ..."
	@/bin/rm -rf $(ETAS_BASE_DIR)/modules/$(obi-ko)
	@echo "Un-installing $(ETAS_BASE_DIR)/modules/$(obi-tar) ..."
	@/bin/rm -rf $(ETAS_BASE_DIR)/modules/$(obi-tar)
	@echo "Un-installing $(DESTDIR)/etc/init.d/$(INSTALL_SCRIPT)..."
	@/bin/rm -rf $(DESTDIR)/etc/init.d/$(INSTALL_SCRIPT) 
	####################### add by ycf
	
####################### add by ycf
tar:
	$(TAR) -czf $(obj-tar)  *.c  *.h  xdma_install
####################### add by ycf


endif
