
## General Flags
PROJECT = main
MCU = atmega168pa
TARGET = $(PROJECT).elf


## Toolchain
#CC = avr-gcc.exe
CC = avr-gcc
UNCRUSTIFY = uncrustify


## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=20000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Included directories
INCLUDES += -I./
INCLUDES += -Iusb/

## Assembly specific flags
ASMFLAGS = $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(PROJECT).map
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
all: $(TARGET) $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss size

## Compile
main.o: main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

ini.o: ini.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

isr.o: isr.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

sram_spi.o: sram_spi.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

usb.o: usb.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

usbdrvasm.o: usb/usbdrvasm.S
	$(CC) $(INCLUDES) $(ASMFLAGS) -c $<

usbdrv.o: usb/usbdrv.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

## Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS) $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean up working directory
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(PROJECT).elf $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss $(PROJECT).map


## Code style
.PHONY: style
style: main.c isr.c ini.c sram_spi.c usb.c
	$(UNCRUSTIFY) -c uncrustify.cfg --replace --no-backup $^


## Other dependencies
#--include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)
