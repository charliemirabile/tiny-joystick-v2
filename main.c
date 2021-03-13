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

	for(;;)
	{		
		usbPoll();
		if(usbInterruptIsReady())
		{
			usbSetInterrupt((uchar[]){0x09,0x90,42,42},4);
		}
	}
}

