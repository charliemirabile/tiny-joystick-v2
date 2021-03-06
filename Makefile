CC = avr-gcc
CFLAGS = -Wall -Wextra -Wpedantic -O2 -mmcu=attiny85 -DF_CPU=16500000
AS = arv-gcc
ASFLAGS = $(CFLAGS)

OBJS = main.o usbconfig.o usbdrv/usbdrv.o usbdrv/usbdrvasm.o

.PHONY: all clean flash

all: main.hex

flash: all
	avrdude -c usbtiny -p attiny85 -B10 -U flash:w:main.hex:i

main.hex: main.bin
	avr-objcopy -j .text -j .data -O ihex $^ $@

main.bin: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm $(OBJS) main.hex main.bin
