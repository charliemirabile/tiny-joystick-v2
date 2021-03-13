/* Mini MIDI Pitchbend Joystick
 * mitxela.com/projects/tiny_joystick
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

#include "usbdrv/usbdrv.h"

void usbFunctionWriteOut(uchar * data, uchar len)
{
}

void main(void)
{
	wdt_disable();

	usbDeviceDisconnect();
	for(uchar i=0;i<250;i++)
	{
		//wdt_reset();
		_delay_ms(2);
	}
	usbDeviceConnect();
	

	usbInit();
	sei();

	ADMUX = (1<<ADLAR) | 0b10; //select reading from PB3
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0);

	for(;;)
	{		
		usbPoll();
		if(usbInterruptIsReady())
		{
			
			ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADSC) | (1<<ADIF); //clear interrupt flag and start conversion
			while(!(ADCSRA&(1<<ADIF)))
				;
			usbSetInterrupt((uchar[]){0x09,0x90,42,0x7F & ADCH},4);
		}
	}
}

