###############################################################################
# Makefile for the project Scanalogic2_test_board
###############################################################################

## General Flags
PROJECT = Scanalogic2_test_board
MCU = atmega168
TARGET = Scanalogic2_test_board.elf
CC = avr-gcc.exe

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99                                            -DF_CPU=20000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=Scanalogic2_test_board.map
LDFLAGS += -Wl,-section-start=.bootloader=0x3800


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = main.o usbdrv.o usbdrvasm.o ini.o isr.o sram_spi.o usb.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) Scanalogic2_test_board.hex Scanalogic2_test_board.eep Scanalogic2_test_board.lss size

## Compile
usbdrvasm.o: ../usbdrvasm.S
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

main.o: ../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usbdrv.o: ../usbdrv.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ini.o: ../ini.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

isr.o: ../isr.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

sram_spi.o: ../sram_spi.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

usb.o: ../usb.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) Scanalogic2_test_board.elf dep/* Scanalogic2_test_board.hex Scanalogic2_test_board.eep Scanalogic2_test_board.lss Scanalogic2_test_board.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

