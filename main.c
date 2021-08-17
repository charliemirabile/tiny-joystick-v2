#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

#include "usbdrv/usbdrv.h"
#include "eeprom.h"
#include "types.h"

#define RUNTIME_MODE_TOGGLE 15

//NOTE: THIS VALUE MUST BE A MULTIPLE OF 8
#define RUNTIME_TYPE_CONFIG_CODE 16

//NOTE: THIS VALUE MUST BE A MULTIPLE OF 8
#define RUNTIME_ARG1_CONFIG_CODE 24

//NOTE: THIS VALUE MUST BE A MULTIPLE OF 8
#define RUNTIME_ARG2_CONFIG_CODE 32

#define EEPROM_CONFIG_CODE 40

#define CHNG_MODE_ENABLE_CODE 44

#define CHNG_MODE_DISABLE_CODE 45

#define PROG_START_CONFIG 46

#define JOYSTICK_RANGE_CONFIG 47

static USB_Msg current_program[8] = {0};

static _Bool mode = 0;

static uint8_t get_pos(void);

static void change_program(uint8_t preset_num);

void usb_function_writeout(uint8_t *data, uint8_t len);

int main(void)
{
	usbDeviceDisconnect();
	for(uint8_t i=0;i<250;i++)
		_delay_ms(2);
	usbDeviceConnect();
	

	usbInit();
	sei();
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1); //enable ADC and set prescaler to divide by 64

	uint8_t last_pos = get_pos();
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

	uint8_t prog=eeprom_read_byte(&prog_start);

	for(;;)
	{
		_Bool main_mode = mode;	
		usbPoll();
		if(usbInterruptIsReady())
		{
			uint8_t pos = get_pos();
			if(pos != last_pos)
			{
				uint8_t move = (0x4 & pos) | (0x3 & (pos | last_pos));
				last_pos = pos;
				if(main_mode==0||move&(LEFT&RIGHT))
				{
					if(current_program[move].usb_header != 0)
					{
						usbSetInterrupt((uint8_t *)&(current_program[move]),sizeof(USB_Msg));
					}
				}
				else
				{
					if(!(move&CENTER))
					{
						prog = 0x7f & (prog + (move?-1:1));
						usbSetInterrupt((uint8_t *)&(USB_Msg){.usb_header=0x0C,.msg={.header=0xC0, .arg1=prog, .arg2=0}},sizeof(USB_Msg));
					}
				}
				
			}
		}
	}
}


static uint8_t get_pos(void)
{
	static uint8_t range = 0;

	//lazily load from eeprom
	if(0==range)
		range = eeprom_read_byte(&joystick_range);

	ADMUX = _BV(ADLAR) | _BV(MUX1) | _BV(MUX0); //select reading from PB3
	ADCSRA |= _BV(ADSC) | _BV(ADIF); //clear interrupt flag and start conversion
	while( !(ADCSRA & _BV(ADIF)) ) //busy loop waiting for conversion to finish
		;

	if(ADCH<128-range)
		return UP;
	if(ADCH>128+range)
		return DOWN;

	ADMUX = _BV(ADLAR) | _BV(MUX1); //select reading from PB4
	ADCSRA |= (1<<ADSC) | (1<< ADIF); //clear interrupt flag and start conversion
	while(!(ADCSRA & (1<<ADIF))) //busy loop waiting for conversion to finish
		;

	if(ADCH<128-range)
		return RIGHT;
	if(ADCH>128+range)
		return LEFT;

	return CENTER;
}

static void change_program(uint8_t preset_num)
{
	for(uint8_t i=0;i<8; ++i)
	{
		eeprom_read_block(&(current_program[i].msg),&(presets[preset_num][i]),sizeof(MIDI_Msg));
		current_program[i].usb_header = current_program[i].msg.header>>4;
	}

	uint16_t mode_storage;
	eeprom_read_block(&mode_storage,&mode_eep,sizeof(mode_eep));
	mode = mode_storage & (1<<preset_num);
}

void usbFunctionWriteOut(uint8_t *data, uint8_t len)
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

	MIDI_Msg *msg = &((USB_Msg*)data)->msg;

	uint8_t prog_index = msg->arg1 & 0x7;
	uint8_t midi_value = 0x7f & msg->arg2;
	uint8_t midi_cntrl = 0x80 | midi_value;
	
	switch(msg->arg1)
	{
#ifdef CHNG_MODE_ENABLE_CODE
	case CHNG_MODE_ENABLE_CODE:
		{
			uint16_t mode_storage;
			eeprom_read_block(&mode_storage,&mode_eep,sizeof(mode_eep));
			mode_storage |= _BV(midi_value&0xf);
			eeprom_update_block(&mode_storage,&mode_eep,sizeof(mode_storage));
		}
		break;
#endif

#ifdef CHNG_MODE_DISABLE_CODE
	case CHNG_MODE_DISABLE_CODE:
		{
			uint16_t mode_storage;
			eeprom_read_block(&mode_storage,&mode_eep,sizeof(mode_eep));
			mode_storage &= ~(_BV(midi_value&0xf));
			eeprom_update_block(&mode_storage,&mode_eep,sizeof(mode_storage));
		}
		break;
#endif

#ifdef EEPROM_CONFIG_CODE
	case EEPROM_CONFIG_CODE+0:
		message_ptr = &presets[msg->arg2>>3][msg->arg2&0x7];
		break;
	case EEPROM_CONFIG_CODE+1:
		if(midi_cntrl>=0xf0)
			eeprom_write_byte(&message_ptr->header,0);
		else
			eeprom_write_byte(&message_ptr->header,midi_cntrl);
		break;
	case EEPROM_CONFIG_CODE+2:
		eeprom_write_byte(&message_ptr->arg1,midi_value);
		break;
	case EEPROM_CONFIG_CODE+3:
		eeprom_write_byte(&message_ptr->arg2,midi_value);
		break;
#endif

#ifdef RUNTIME_ARG1_CONFIG_CODE
	case RUNTIME_ARG1_CONFIG_CODE + 0:
	case RUNTIME_ARG1_CONFIG_CODE + 1:
	case RUNTIME_ARG1_CONFIG_CODE + 2:
	case RUNTIME_ARG1_CONFIG_CODE + 3:
	case RUNTIME_ARG1_CONFIG_CODE + 4:
	case RUNTIME_ARG1_CONFIG_CODE + 5:
	case RUNTIME_ARG1_CONFIG_CODE + 6:
	case RUNTIME_ARG1_CONFIG_CODE + 7:
		current_program[prog_index].msg.arg1=midi_value;
		break;
#endif

#ifdef RUNTIME_ARG2_CONFIG_CODE
	case RUNTIME_ARG2_CONFIG_CODE + 0:
	case RUNTIME_ARG2_CONFIG_CODE + 1:
	case RUNTIME_ARG2_CONFIG_CODE + 2:
	case RUNTIME_ARG2_CONFIG_CODE + 3:
	case RUNTIME_ARG2_CONFIG_CODE + 4:
	case RUNTIME_ARG2_CONFIG_CODE + 5:
	case RUNTIME_ARG2_CONFIG_CODE + 6:
	case RUNTIME_ARG2_CONFIG_CODE + 7:
		current_program[prog_index].msg.arg2=midi_value;
		break;
#endif

#ifdef RUNTIME_TYPE_CONFIG_CODE
	case RUNTIME_TYPE_CONFIG_CODE + 0:
	case RUNTIME_TYPE_CONFIG_CODE + 1:
	case RUNTIME_TYPE_CONFIG_CODE + 2:
	case RUNTIME_TYPE_CONFIG_CODE + 3:
	case RUNTIME_TYPE_CONFIG_CODE + 4:
	case RUNTIME_TYPE_CONFIG_CODE + 5:
	case RUNTIME_TYPE_CONFIG_CODE + 6:
	case RUNTIME_TYPE_CONFIG_CODE + 7:
		if(midi_cntrl>=0xF0)
		{
			current_program[prog_index].usb_header=0;
		}
		else
		{
			current_program[prog_index].msg.header=midi_cntrl;
			current_program[prog_index].usb_header=midi_cntrl>>4;
		}
		break;
#endif

#ifdef RUNTIME_MODE_TOGGLE
	case RUNTIME_MODE_TOGGLE:
		mode = !mode;
		break;
#endif

#ifdef PROG_START_CONFIG
	case PROG_START_CONFIG:
		eeprom_update_byte(&prog_start,midi_value);
		break;
#endif

#ifdef JOYSTICK_RANGE_CONFIG
	case JOYSTICK_RANGE_CONFIG:
		eeprom_update_byte(&joystick_range, midi_value);
		break;
#endif
	}
}
