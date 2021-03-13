DEVICE = attiny85
PROGRAMMER = usbtiny

CC = avr-gcc
CFLAGS = -Wall -Wextra -Wpedantic -O2 -mmcu=$(DEVICE) -DF_CPU=16500000
AS = arv-gcc
ASFLAGS = $(CFLAGS)
AVRDUDE = avrdude
AVRFLAGS = -c $(PROGRAMMER) -p $(DEVICE) -B10

OBJS = main.o usbconfig.o usbdrv/usbdrv.o usbdrv/usbdrvasm.o

.PHONY: all program fuses flash eeprom dump clean

all: main.bin

program: fuses flash eeprom

fuses:
	$(AVRDUDE) $(AVRFLAGS) -U hfuse:w:0xD7:m -U lfuse:w:0xE1:m

flash: main.hex
	$(AVRDUDE) $(AVRFLAGS) -U $@:w:$^:i

eeprom: main.eep
	$(AVRDUDE) $(AVRFLAGS) -U $@:w:$^:i

dump:
	$(AVRDUDE) $(AVRFLAGS) -U flash:r:backup_flash.bin:r
	$(AVRDUDE) $(AVRFLAGS) -U eeprom:r:backup_eep.bin:r
	
main.hex: main.bin
	avr-objcopy -j .text -j .data -O ihex $^ $@

main.eep: main.bin
	avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex $^ $@

main.bin: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm $(OBJS) main.bin main.hex main.eep
