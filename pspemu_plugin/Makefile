TARGET = ge_patch
OBJS = main.o gu.o exports.o

SYSCTRL_ROOT := ../third_party/psp_systemctrl

CFLAGS = -O3 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CFLAGS   += -I$(SYSCTRL_ROOT)/include
LDFLAGS  += -L$(SYSCTRL_ROOT)/lib
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS += -lpspsystemctrl_kernel

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak