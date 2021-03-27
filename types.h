#ifndef TYPES_H
#define TYPES_H
typedef enum
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	CENTER,
}
Position;

typedef struct
{
	uint8_t header;
	uint8_t arg1;
	uint8_t arg2;
}
MIDI_Msg;

typedef MIDI_Msg Preset[8];

typedef struct
{
	uint8_t usb_header;
	MIDI_Msg msg;
}
USB_Msg;
#endif//TYPES_H
