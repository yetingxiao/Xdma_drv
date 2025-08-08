# COMPILER=gcc
# ARCHIVER=
# CP=cp
# COMPILER_FLAGS= -rdynamic -shared -fPIC

# LIB=libxilAXIUart.so

# INCLUDEFILES=xuartns550.h xuartns550_l.h
# RELEASEDIR=../../lib
# INCLUDEDIR=./include
# INCLUDES= -I${INCLUDEDIR}

# XVC_FLAGS= $(CC_ARCH) -rdynamic -fPIC
# EXTRA_CFLAGS := -I$(INCLUDEDIR) 

# LIBSOURCES=$(wildcard *.c)
# OBJECTS =	$(addsuffix .o, $(basename $(wildcard *.c)))
# TARGET_MODULE:=xilAXIUart
# $(TARGET_MODULE)-objs :=$(OBJECTS)
# EXTRA_COMPILER_FLAGS= 
# all:
	# echo "Compiling uartns550,Making UART 16550 Shared lib"
	# $(COMPILER) $(COMPILER_FLAGS) $(INCLUDE_DIRS) $(LIBSOURCES) -o $(LIB) 
	#$(COMPILER) $(COMPILER_FLAGS) $(INCLUDE) $(LIBSOURCES) -o $(LIB) 
	#$(MAKE) -C $(EXTRA_CFLAGS) $(LINUX_DIR) M=$(PWD) modules

# clean:
	# rm -rf ${LIB}
	
# 定义自己的编译选项
CC = gcc
CFLAGS =  $(CC_ARCH) -shared -rdynamic -fPIC
#CFLAGS =  $(CC_ARCH) -static  -fPIC
#ccflags-y +=$(CC_ARCH) -static  -fPIC
# 指定自己的头文件搜索路径
#INCLUDES = -I/usr/src/linux-headers-4.9.178-rt131-1/include/
INCLUDES = -I/usr/include/
# 假设您的内核头文件根目录是这个
KERNEL_HEADERS_ROOT = /usr/src/linux-headers-4.9.178-rt131-1

# 添加架构特定的include路径
# 注意：这里需要根据你的目标架构（例如x86）来指定
# 如果是x86，就是 arch/x86/include
# 如果是arm，就是 arch/arm/include
ARCH_INCLUDE_PATH = $(KERNEL_HEADERS_ROOT)/arch/x86/include

# 将这个路径添加到编译器的头文件搜索路径中
# 通常通过 -I 选项实现
# 如果你正在编译内核模块，可能需要使用 EXTRA_CFLAGS
#CFLAGS += -I$(ARCH_INCLUDE_PATH)

# 你的源文件和目标文件
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
#TARGET = libxilAXIUart.so
TARGET = libxilAXIUart.a

.PHONY: all clean

all: $(TARGET)
#all: $(OBJS)
$(TARGET): $(OBJS)
	#$(MAKE) -C $(KERNEL_HEADERS_ROOT)  $(LINUX_DIR) M=$(PWD)  $(INCLUDES) modules
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

modules:$(OBJS)
	#$(MAKE) -C $(LINUX_DIR) M=$(PWD)  $(INCLUDES) modules
	#$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# 编译.c文件到.o文件
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)


