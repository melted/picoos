#
#  This makefile demonstrates how you can use your own (standard GNU) makefile
#  to build your application. The Pico]OS library is re-build automatically
#  when the configuration files have changed (see rule "$(RTOS)" ).
#  The pico]OS library is stored in a subdirectory called "rtos".
#
#  This makefile was written for Windows XP and the MinGW compiler.
#


# set relative path to pico]OS root directory
PICOOS_PATH =  ../../picoos

# set this to 1 if you whish to use the nano layer
USE_NANOLAYER =  1

# not nice but required by MinGW: Set the correct path here
MINGW_LIB_DIR =  E:/Programme/mingw/lib

# Get current directory (on a Windows XP machine using cmd.exe, no cygwin)
CURRENTDIR =  $(subst \,/,$(shell cd))

# Some special pico]OS settings (C-includes, C-defines)
CINCLUDES = $(PICOOS_PATH)/inc $(PICOOS_PATH)/ports/x86w32 
ifeq '$(strip $(USE_NANOLAYER))' '1'
CDEFINES += POSNANO
endif



#----------------------------------------------------------------------------
#  standard makefile:


OBJ     = memtest.o
RTOS    = rtos/picoos.a
TARGET  = demo2.exe

CC      = gcc
LD      = gcc
CFLAGS  = -Wall -g -I. $(addprefix -I,$(CINCLUDES)) $(addprefix -D,$(CDEFINES))
LDFLAGS = -lm


default: all

all: $(OBJ) $(RTOS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJ) $(RTOS) $(MINGW_LIB_DIR)/libwinmm.a


$(RTOS): poscfg.h noscfg.h
	$(MAKE) \
	    -C $(PICOOS_PATH) $(MAKECMDGOALS) NANO=$(USE_NANOLAYER) PORT=x86w32 \
	    RTOS_OUTPUTDIR=$(CURRENTDIR)/rtos DIR_CONFIG=$(CURRENTDIR) COMPILER=GCC


%.o: %.c
	$(CC) $(CFLAGS) -c $<
