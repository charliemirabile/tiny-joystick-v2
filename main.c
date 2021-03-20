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

typedef struct
{
	uchar header;
	uchar arg1;
	uchar arg2;
}
MIDI_Msg;

typedef MIDI_Msg Preset[8];

EEMEM Preset presets[16];

typedef struct
{
	uchar usb_header;
	MIDI_Msg msg;
}
USB_Msg;


static EEMEM uint16_t mode_eep = 0;
static uint16_t mode_storage = 0;

static USB_Msg current_program[8] = {0};

static _Bool mode = 0;

void change_program(uchar preset_num)
{
	for(uchar i=0;i<8; ++i)
	{
		eeprom_read_block(&(current_program[i].msg),&(presets[preset_num][i]),sizeof(MIDI_Msg));
		current_program[i].usb_header = current_program[i].msg.header>>4;
	}
	mode = mode_storage & (1<<preset_num);
}

#define MODE_TOGGLE_CODE 15
#define RUNTIME_TYPE_CONFIG_CODE 16
#define RUNTIME_ARG1_CONFIG_CODE 24
#define RUNTIME_ARG2_CONFIG_CODE 32
#define EEPROM_CONFIG_CODE 40
#define RUNTIME_MODE_TOGGLE 45

void usbFunctionWriteOut(uchar * data, uchar len)
{
	static MIDI_Msg *message_ptr = &(presets[0][0]);

	//program change
	if(data[0] == 0x0C && data[1] == 0xC0)
	{
		change_program(data[2]&0xf);
		return;
	}

	if(len != 4)
		return;
	if(data[0] != 0x0B)
		return;
	if(data[1] != 0xB0)
		return;
	
	switch(data[2])
	{
#ifdef MODE_TOGGLE_CODE
	case MODE_TOGGLE_CODE:
		mode_storage ^= 1 << (data[2]&0xf);
		eeprom_update_block(&mode_storage,&mode_eep,sizeof(mode_storage));
		break;
#endif

#ifdef RUNTIME_MODE_TOGGLE
	case RUNTIME_MODE_TOGGLE:
		mode = !mode;
		break;
#endif

#ifdef EEPROM_CONFIG_CODE
	case EEPROM_CONFIG_CODE+0:
		message_ptr = &presets[data[3]>>3][data[3]&0x7];
		break;
	case EEPROM_CONFIG_CODE+1:
		if(data[3]>=0x70)
			eeprom_write_byte(&message_ptr->header,0);
		else
			eeprom_write_byte(&message_ptr->header,0x80|data[3]);
		break;
	case EEPROM_CONFIG_CODE+2:
		eeprom_write_byte(&message_ptr->arg1,data[3]);
		break;
	case EEPROM_CONFIG_CODE+3:
		eeprom_write_byte(&message_ptr->arg2,data[3]);
		break;
#endif

#ifdef RUNTIME_ARG1_CONFIG_CODE
	case RUNTIME_ARG1_CONFIG_CODE ... RUNTIME_ARG1_CONFIG_CODE+7:
		current_program[data[2]-RUNTIME_ARG1_CONFIG_CODE].msg.arg1=data[3];
		break;
#endif

#ifdef RUNTIME_ARG2_CONFIG_CODE
	case RUNTIME_ARG2_CONFIG_CODE ... RUNTIME_ARG2_CONFIG_CODE+7:
		current_program[data[2]-RUNTIME_ARG2_CONFIG_CODE].msg.arg2=data[3];
		break;
#endif

#ifdef RUNTIME_TYPE_CONFIG_CODE
	case RUNTIME_TYPE_CONFIG_CODE ... RUNTIME_TYPE_CONFIG_CODE+7:
		if(data[3]>=0x70)
		{
			current_program[data[2]-RUNTIME_TYPE_CONFIG_CODE].usb_header=0;
		}
		else
		{
			current_program[data[2]-RUNTIME_TYPE_CONFIG_CODE].msg.header=0x80|data[3];
			current_program[data[2]-RUNTIME_TYPE_CONFIG_CODE].usb_header=(0x80|data[3])>>4;
		}
		break;
#endif
		
	}
}

typedef enum
{
	CENTER,
	UP,
	DOWN,
	LEFT,
	RIGHT,
}
Position;

uchar get_pos(void)
{
	ADMUX = (1<<ADLAR) | 0b11; //select reading from PB3

	ADCSRA |= (1<<ADSC) | (1 << ADIF); //clear interrupt flag and start conversion

	while(!(ADCSRA & (1<<ADIF))) //busy loop waiting for conversion to finish
		;

	if(ADCH<32)
		return UP;

	if(ADCH>224)
		return DOWN;

	ADMUX = (1<<ADLAR) | 0b10; //select reading from PB4

	ADCSRA |= (1<<ADSC) | (1<< ADIF); //clear interrupt flag and start conversion

	while(!(ADCSRA & (1<<ADIF))) //busy loop waiting for conversion to finish
		;

	if(ADCH<32)
		return LEFT;

	if(ADCH>224)
		return RIGHT;

	return CENTER;
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
	ADCSRA = 1 << ADEN | 0b110; //enable ADC and set prescaler to 6 (divide by 64)

	eeprom_read_block(&mode_storage,&mode_eep,sizeof(mode_storage));

	uchar last_pos = get_pos();
	switch(last_pos)
	{
	case CENTER:
		change_program(0);
		break;
	case UP:
		change_program(1);
		break;
	case DOWN:
		change_program(2);
		break;
	case LEFT:
		change_program(3);
		break;
	case RIGHT:
		change_program(4);
		break;
	}

	uchar prog=0;

	for(;;)
	{		
		usbPoll();
		if(usbInterruptIsReady())
		{
			uchar pos = get_pos();
			if(pos != last_pos)
			{
				uchar move;
				if(last_pos)
					move=last_pos+3;
				else
					move=pos-1;
				last_pos = pos;
				if(mode==0||move&2)
				{
					if(current_program[move].usb_header != 0)
						usbSetInterrupt((uchar *)&(current_program[move]),sizeof(USB_Msg));
				}
				else
				{
					if(!(move&4))
					{
						if(move)
							--prog;
						else
							++prog;
						prog&=127;
						usbSetInterrupt((uchar *)&(USB_Msg){.usb_header=0x0C,.msg={.header=0xC0, .arg1=prog, .arg2=0}},sizeof(USB_Msg));
					}
				}
				
			}
		}
	}
}
