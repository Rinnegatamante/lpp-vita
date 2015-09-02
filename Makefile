TARGET		:= lpp-vita
SOURCES		:= source/include/lua source 
INCLUDES	:= include

LIBS = -lSceKernel_stub -lSceDisplay_stub -lSceGxm_stub	\
	-lSceCtrl_stub -lSceTouch_stub -lm

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.c=.o) 

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CFLAGS  = -Wl,-q -Wall -O3 -I$(INCLUDES)
ASFLAGS = $(CFLAGS)

all: $(TARGET).velf

%.velf: %.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@ $(VITASDK)/bin/db.json

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS)

copy: $(TARGET).velf
	@cp $(TARGET).velf ~/shared/vitasample.elf
	@echo "Copied!"

run: $(TARGET).velf
	@sh run_homebrew_unity.sh $(TARGET).velf