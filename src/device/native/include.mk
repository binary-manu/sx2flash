# Tunables
FLASH_SIZE       := 32768
FLASH_PAGE_SIZE  := 128

DEVICE_CPPFLAGS  := -D_POSIX_C_SOURCE=200809L -DFLASH_SIZE=$(FLASH_SIZE) -DFLASH_PAGE_SIZE=$(FLASH_PAGE_SIZE)
DEVICE_CFLAGS    := -g3 -O0
DEVICE_C_SOURCES := flash.c payload.c timer.c uart.c

CC               := gcc
AS               := as
OBJCOPY          := # No hex file for the native version