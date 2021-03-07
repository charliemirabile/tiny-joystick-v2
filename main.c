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






//uchar note = 0;

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
current_program = {.direction_lookup_table[0] = {.packet_header = 0x09, .midi_header = 0x90, .midi_arg1 = 42, .midi_arg2 = 42}};

typedef union
{
	uchar byte;
	
	struct
	{
		uchar direction:3;
		uchar preset_num:4;
	};
	struct
	{
		uchar channel:4;
		uchar type:3;
	};
}
config_byte;

typedef union
{
	uchar *ptr;
	uint16_t addr;
	struct
	{
		uint16_t offset:2;
		uint16_t direction:3;
		uint16_t preset:4;
	};
	struct
	{
		uint16_t _:2;
		uint16_t index:7;
	};
}
config_loc;

#define NOTE_OFF 0x8
#define NOTE_ON 0x9
#define POLY_PRESS 0xA
#define CONTROLLER 0xB
#define PRG_CHANGE 0xC
#define CHAN_PRESS 0xD
#define PITCH_BEND 0xE
#define NO_MSG 0xF



static union
{
	uchar lookup_table[8][4];
	uchar bytes[32];
}
current_prog = {.bytes = {0}};

void change_program(uchar prog)
{
	config_loc loc = {.preset = prog & 0xf};//only 16 presets possible

	//copy the 32 byte table for the preset into ram
	for(uchar i=0;i<32;++i)
		current_program.bytes[i]=eeprom_read_byte(loc.ptr+i);	
}

static _Bool toggle_mode = 0;


#define RUNTIME_TYPE_CONFIG_CODE 16
#define RUNTIME_ARG1_CONFIG_CODE 24
#define RUNTIME_ARG2_CONFIG_CODE 32
#define EEPROM_CONFIG_CODE 40
#define MODE_SWAP_CODE 15

void usbFunctionWriteOut(uchar * data, uchar len)
{
	static config_loc loc = {.addr=0};

	//if its not a controller message on virt cable 0 bail
	if(len != 4)
		return;

	//program change
	if(data[0] == 0x0C && data[1] == 0xC0)
	{
		change_program(data[2]);
		return;
	}
	if(data[0] != 0x0B)
		return;
	if(data[1] != 0xB0)
		return;

	uchar config = data[3];

	uchar type = data[3]|0x80;
	
	uchar usb_midi_header = type>>4;
	
	switch(data[2])
	{
#ifdef MODE_SWAP_CODE
	case MODE_SWAP_CODE:
		toggle_mode = 1;
		break;
#endif
#ifdef EEPROM_CONFIG_CODE
	case EEPROM_CONFIG_CODE+0:
		loc.index = config;
		break;
	case EEPROM_CONFIG_CODE+1:
		//check for them wanting to send no message
		if(0xf == usb_midi_header)
		{
			//write 0 to indicate no message
			eeprom_write_byte(loc.ptr,0);
		}
		else
		{
			//write the usb midi header byte
			eeprom_write_byte(loc.ptr,usb_midi_header);
			//write the midi header byte
			eeprom_write_byte(loc.ptr+1, type);
		}
		break;
	case EEPROM_CONFIG_CODE+2:
		//write the first argument byte
		eeprom_write_byte(loc.ptr+2,config);
		break;
	case EEPROM_CONFIG_CODE+3:
		//write the second argument byte
		eeprom_write_byte(loc.ptr+3,config);
		break;
#endif

#ifdef RUNTIME_ARG1_CONFIG_CODE
	case RUNTIME_ARG1_CONFIG_CODE ... RUNTIME_ARG1_CONFIG_CODE+7:
		current_program.direction_lookup_table[data[2]-RUNTIME_ARG1_CONFIG_CODE].bytes[2] = config;
		break;
#endif

#ifdef RUNTIME_ARG2_CONFIG_CODE
	case RUNTIME_ARG2_CONFIG_CODE ... RUNTIME_ARG2_CONFIG_CODE+7:
		current_program.direction_lookup_table[data[2]-RUNTIME_ARG2_CONFIG_CODE].bytes[3] = config;
		break;
#endif

#ifdef RUNTIME_TYPE_CONFIG_CODE
	case RUNTIME_TYPE_CONFIG_CODE ... RUNTIME_TYPE_CONFIG_CODE+7:
		if(0xf == usb_midi_header)
		{
			current_program.direction_lookup_table[data[2]-RUNTIME_TYPE_CONFIG_CODE].bytes[0] = 0;
		}
		else
		{
			current_program.direction_lookup_table[data[2]-RUNTIME_TYPE_CONFIG_CODE].bytes[0] = usb_midi_header;
			current_program.direction_lookup_table[data[2]-RUNTIME_TYPE_CONFIG_CODE].bytes[1] = type;
		}
		break;
#endif
		
	}
/*
	if(data[0]==0x0B && data[1]==0xB0 && data[2]==100 && data[3]==0)
	{
		++note;
		if(note==128)
			note = 0;
	}
*/}

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

typedef union
{
	uchar bytes[4];
	struct
	{
		uchar codeindex:4;
		uchar cablenum:4;
		uchar channel:4;
		uchar msg_type:4;
		union
		{
			uchar note:7;
			uchar controller:7;
			uchar program:7;
			uchar chan_pressure:7;
			uchar bend_lsb:7;
		};
		union
		{
			uchar velocity:7;
			uchar pressure:7;
			uchar value:7;
			uchar bend_msb:7;
		};
	};	
}
midimsg;



midimsg lookuptable[] = {
	(midimsg){.codeindex=0xB, .channel=0, .msg_type=0xB, .controller=100, .value=100},
	(midimsg){.codeindex=0xB, .channel=0, .msg_type=0xB, .controller=101, .value=100},
	(midimsg){.codeindex=0xB, .channel=0, .msg_type=0xB, .controller=102, .value=100},
	(midimsg){.codeindex=0xB, .channel=0, .msg_type=0xB, .controller=103, .value=100},
	(midimsg){.codeindex=0xB, .channel=0, .msg_type=0xB, .controller=100, .value=0},
	(midimsg){.codeindex=0xB, .channel=0, .msg_type=0xB, .controller=101, .value=0},
	(midimsg){.codeindex=0xB, .channel=0, .msg_type=0xB, .controller=102, .value=0},
	(midimsg){.codeindex=0xB, .channel=0, .msg_type=0xB, .controller=103, .value=0},
};


/*
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
}*/



//////// Main ////////////
void main(void)
{
	_Bool mode = 0;
	uchar prog = 0;
	uchar last_pos = CENTER;
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

	for(;;)
	{		
		usbPoll();
		if(usbInterruptIsReady())
		{
			if(toggle_mode)
			{
				toggle_mode=0;
				mode = !mode;
			}
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
					if(current_program.direction_lookup_table[move].bytes[0] != 0)
						usbSetInterrupt(current_program.direction_lookup_table[move].bytes,sizeof(USB_midi_msg));
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
						usbSetInterrupt((USB_midi_msg){.packet_header=0x0C,.midi_header=0xC0,.midi_arg1=prog, .midi_arg2=0}.bytes,sizeof(USB_midi_msg));
					}
				}
				
			}
		}

	}
}

