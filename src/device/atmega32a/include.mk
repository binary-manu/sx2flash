# Tunables
F_CPU              := 1000000
RESET_ADDR         := 0x7000
BAUD_RATE          := 9600

DEVICE_CPPFLAGS    := -DBAUD_RATE=$(BAUD_RATE) -DF_CPU=$(F_CPU)
DEVICE_CFLAGS      := -mmcu=$(DEVICE) -xc -Os -mcall-prologues
DEVICE_ASFLAGS     := -mmcu=$(DEVICE)
DEVICE_LDFLAGS     := -mmcu=$(DEVICE) -Wl,-Ttext=$(RESET_ADDR)

DEVICE_C_SOURCES   := flash.c payload.c timer.c uart.c
DEVICE_ASM_SOURCES := spm.S

CC                 := avr-gcc
AS                 := avr-gcc
OBJCOPY            := avr-objcopy
AVRDUDE            := avrdude
PROGRAMMER         := -cusbasp-clone

.PHONY : flash erase 

flash: $(HEX_FILE)
	sudo '$(AVRDUDE)' -p$(DEVICE) $(PROGRAMMER) -U 'flash:w:$<:i'

erase:
	sudo '$(AVRDUDE)' -p$(DEVICE) $(PROGRAMMER) -e