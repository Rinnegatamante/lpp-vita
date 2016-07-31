TARGET		:= lpp-vita
TITLE		:= LPPV000001
SOURCES		:= source/include/lua source/include/ftp source/include source source/include/draw
INCLUDES	:= include

LIBS = -lvita2d -lSceKernel_stub -lSceDisplay_stub -lSceGxm_stub	\
	-lSceSysmodule_stub -lSceCtrl_stub -lSceTouch_stub -lm -lSceNet_stub \
	-lSceNetCtl_stub -lScePgf_stub -ljpeg -lfreetype -lc \
	-lScePower_stub -lSceCommonDialog_stub -lpng -lz

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -Wl,-q -Wall -O3
CXXFLAGS  = $(CFLAGS) -fno-exceptions
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
    vita-make-fself $< eboot.bin
    vita-mksfoex -s TITLE_ID=$(TITLE) "Lua Player+" param.sfo
    vita-pack-vpk -s param.sfo -b eboot.bin $(TARGET).vpk
	
%.velf: %.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS) param.sfo eboot.bin $(TARGET).vpk
