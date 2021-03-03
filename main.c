#include <util/delay.h>
#include <avr/interrupt.h>
#include "usbdrv/usbdrv.h"

typedef enum
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	CENTER,
}
Position;

uchar get_pos(void)
{
	ADMUX = (1<<ADLAR) | 0x3; //select reading from PB3

	ADCSRA |= (1<<ADSC) | (1 << ADIF); //clear interrupt flag and start conversion
	while(!(ADCSRA & (1<<ADIF))) //busy loop waiting for conversion to finish
		;

	if(ADCH<32)
		return UP;
	if(ADCH>224)
		return DOWN;

	ADMUX = (1<<ADLAR) | 0x2; //select reading from PB4

	ADCSRA |= (1<<ADSC) | (1<< ADIF); //clear interrupt flag and start conversion
	while(!(ADCSRA & (1<<ADIF))) //busy loop waiting for conversion to finish
		;

	if(ADCH<32)
		return LEFT;
	if(ADCH>224)
		return RIGHT;

	return CENTER;
}


typedef union
{
	uchar bytes[4];
	struct
	{
		uchar packet_header;
		uchar midi_header;
		uchar midi_arg1;
		uchar midi_arg2;
	};
}
USB_midi_msg;

static union
{
	uchar bytes[32];
	USB_midi_msg direction_lookup_table[8];	
}
current_program = {.bytes = {0}};

void usbFunctionWriteOut(uchar *data, uchar len)
{
	(void)data;
	(void)len;
}

int main(void)
{
	usbDeviceDisconnect();
	for(uchar i=0;i<250;i++)
		_delay_ms(2);
	usbDeviceConnect();

	usbInit();
	sei();

	uchar last_pos = CENTER;

	for(;;)
	{
		usbPoll();
		if(usbInterruptIsReady())
		{
			uchar pos = get_pos();
			if(pos != last_pos)
			{
				uchar move = pos & 0x4; //if new position is center we set the 4's place
				move |= (pos | last_pos) & 0x3; //1's and 2's place comes from the direction
				last_pos = pos;
				if(current_program.direction_lookup_table[move].bytes[0] != 0)
					usbSetInterrupt(current_program.direction_lookup_table[move].bytes,sizeof(USB_midi_msg));
			}
		}
	}
}
