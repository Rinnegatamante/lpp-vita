TARGET		:= lpp-vita
SOURCES		:= source/include/lua source/include/ftp source/include source \
			source/include/audiodec
INCLUDES	:= include

LIBS = -lvorbisfile -logg -lvorbis -lsndfile -lvita2d -lSceKernel_stub \
	-lSceAppMgr_stub -lSceSysmodule_stub -lSceCtrl_stub -lSceTouch_stub \
	-lm -lSceNet_stub -lSceNetCtl_stub -lSceAppUtil_stub -lScePgf_stub \
	-ljpeg -lfreetype -lc -lScePower_stub -lSceCommonDialog_stub -lpng16 -lz \
	-lspeexdsp -lmpg123 -lSceAudio_stub -lSceGxm_stub -lSceDisplay_stub

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -g -Wl,-q -O3 -DWANT_FASTWAV -DHAVE_LIBSPEEXDSP \
		-DHAVE_LIBSNDFILE -DHAVE_MPG123 -DWANT_FMMIDI=1 \
		-DUSE_AUDIO_RESAMPLER -DHAVE_OGGVORBIS
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11 -fpermissive
ASFLAGS = $(CFLAGS)

all: $(TARGET).velf

%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@
	vita-make-fself $@ eboot.bin

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS)
