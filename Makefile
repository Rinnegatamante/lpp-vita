TARGET		:= lpp-vita
SOURCES	:= source/include/ftp source/include source source/include/audiodec

LIBS = -lcurl -lssl -lcrypto -lvorbisfile -lvorbis -logg -lsndfile -lvita2d -lSceLibKernel_stub -lScePvf_stub \
	-limgui_vita2d -lSceJpegEnc_stub -lSceAppMgr_stub -lSceCtrl_stub -lSceTouch_stub -lSceMotion_stub \
	-lScePromoterUtil_stub -lm -lSceNet_stub -lSceNetCtl_stub -lSceAppUtil_stub -lScePgf_stub \
	-ljpeg -lfreetype -lc -lScePower_stub -lSceCommonDialog_stub -lpng16 -lz -lSceCamera_stub \
	-lspeexdsp -lmpg123 -lSceAudio_stub -lSceGxm_stub -lSceDisplay_stub -lSceShellSvc_stub -limagequant \
	-lopusfile -lFLAC -lvorbis -lvorbisenc -lopus -lSceHttp_stub -lSceAudioIn_stub -lluajit-5.1 -ldl \
	-ltaihen_stub -lSceKernelModulemgr_stub -lSceSblSsMgr_stub  -lSceSysmodule_stub -lSceShutterSound_stub \
	-lSceSsl_stub -lSceVshBridge_stub -lSceAvPlayer_stub -lSceRegistryMgr_stub

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -fno-lto -g -Wl,-q -O3 -DWANT_FASTWAV -DHAVE_LIBSPEEXDSP \
		-DHAVE_LIBSNDFILE -DHAVE_MPG123 -DWANT_FMMIDI=1 -DWANT_FASTAIFF \
		-DUSE_AUDIO_RESAMPLER -DHAVE_OGGVORBIS -DHAVE_OPUSFILE \
		-DSQLITE_OS_OTHER=1 -DSQLITE_TEMP_STORE=3 -DSQLITE_THREADSAFE=0 \
		-I$(VITASDK)/$(PREFIX)/include/opus -I$(VITASDK)/$(PREFIX)/include/luajit-2.1

ifeq ($(SYS_APP_MODE),1)
CFLAGS += -DSYS_APP_MODE
endif

CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11 -fpermissive
ASFLAGS = $(CFLAGS)

ifeq ($(SYS_APP_MODE),1)
all: $(TARGET).sysvelf
else
all: $(TARGET).velf
endif

%.sysvelf: %.elf
	cp $< $<.unstripped_sys.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@
	vita-make-fself -c -at 0x0E -m 0x12800 -pm 0x8000 -a 0x2800000000000001 $@ eboot_unsafe_sys.bin

%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@
	vita-make-fself -c -a 0x2800000000000001 $@ eboot_unsafe.bin
	vita-make-fself -c -s $@ eboot_safe.bin

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(TARGET).sysvelf $(OBJS)
