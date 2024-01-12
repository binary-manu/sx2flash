# Select the initial delay before booting the payload, in milliseconds.
# Please ensure that this number is supported by the timer code for the device.
# For the ATMEGA32, we use a 1024 prescaler, so at 1MHz we can count approximately
# 65535 ms. This value reduces when F_CPU increases. AT 16MHz, we can wait at most 4s.
XMODEM_TIMEOUT           := 2000
# Largest amount of bytes supported in a single Intel Hex record. This refers to the
# raw binary data, not the number of characters used to represent them, which is double.
IHEX_MAX_DATA_PER_RECORD := 16

TOPDIR      := $(dir $(lastword $(MAKEFILE_LIST)))
DEVICE      := atmega32a
FILENAME    := sx2flash
C_SOURCES   := main.c xmodem.c ihex.c
C_SOURCES   := $(addprefix common/,$(C_SOURCES))
ASM_SOURCES := # EMpty but declares ASM_SOURCES as simply defined
EXT_C       := .c
EXT_ASM     := .S
EXT_OBJ     := .o
EXT_DEP     := .d
EXT_ELF     := .elf
EXT_HEX     := .hex
ELF_FILE    := $(FILENAME)$(EXT_ELF)
HEX_FILE    := $(FILENAME)$(EXT_HEX)
CPPFLAGS    := -I$(TOPDIR)common -I$(TOPDIR)device/$(DEVICE) -DXMODEM_TIMEOUT=$(XMODEM_TIMEOUT) \
               -DIHEX_MAX_DATA_PER_RECORD=$(IHEX_MAX_DATA_PER_RECORD)
CFLAGS      := -std=gnu17 -c -Woverflow
ASFLAGS     := -c

.PHONY: all build clean

all: build

include $(TOPDIR)device/$(DEVICE)/include.mk

CPPFLAGS    += $(DEVICE_CPPFLAGS)
CFLAGS      += $(DEVICE_CFLAGS)
LDFLAGS     += $(DEVICE_LDFLAGS)
ASFLAGS     += $(DEVICE_ASFLAGS)
C_SOURCES   += $(addprefix device/$(DEVICE)/,$(DEVICE_C_SOURCES))
ASM_SOURCES += $(addprefix device/$(DEVICE)/,$(DEVICE_ASM_SOURCES))

SOURCES     := $(C_SOURCES) $(ASM_SOURCES)
OBJECTS     := $(addsuffix $(EXT_OBJ),$(basename $(SOURCES)))
VPATH       := $(TOPDIR)

-include $(addsuffix $(EXT_DEP),$(basename $(C_SOURCES)))

ifneq "$(OBJCOPY)" ""
build: $(HEX_FILE)

%$(EXT_HEX) : %$(EXT_ELF)
	$(call ensure_dir,$@)
	'$(OBJCOPY)' -O ihex $< $@
else
build: $(ELF_FILE)
endif

ensure_dir = @mkdir -p $(dir $1)

$(ELF_FILE) : $(OBJECTS)
	$(call ensure_dir,$@)
	'$(CC)' $(LDFLAGS) -o $@ $^


%$(EXT_OBJ) : %$(EXT_C)
	$(call ensure_dir,$@)
	'$(CC)' $(CPPFLAGS) $(CFLAGS) -o $@ $<

%$(EXT_OBJ) : %$(EXT_ASM)
	$(call ensure_dir,$@)
	'$(AS)' $(CPPFLAGS) $(ASFLAGS) -o $@ $<

# Adapter from the GNU Make manual
%$(EXT_DEP): %$(EXT_C)
	$(call ensure_dir,$@)
	@set -e; rm -f $@; \
	'$(CC)' -MM $(CPPFLAGS) $< > $@.$$$$; \
 	sed '1s,\(.*\),$@ $(dir $@)\1,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	find \( -name *$(EXT_HEX) -o -name *$(EXT_ELF) -o -name *$(EXT_OBJ) -o -name *$(EXT_DEP) -o -empty \) -delete