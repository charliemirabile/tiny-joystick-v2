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

_Bool toggle=0;

void usbFunctionWriteOut(uchar * data, uchar len)
{
	if(data[0]=0x0B && data[1]==0xB0 && data[2]==100 && data[3]==100)
		toggle=1;
}

void main(void)
{
	_Bool blast = 1;
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

	for(;;)
	{		
		usbPoll();
		if(usbInterruptIsReady())
		{
			if(toggle)
			{
				toggle=0;
				blast = !blast;
			}
			if(blast)
				usbSetInterrupt((uchar[]){0x09,0x90,42,42},4);
		}
	}
}

