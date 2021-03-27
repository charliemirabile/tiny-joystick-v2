#include "eeprom.h"
#include <avr/eeprom.h>

EEMEM uint16_t mode_eep = 0;

EEMEM uint8_t prog_start = 0;

EEMEM uint8_t joystick_range = 96;

EEMEM Preset presets[16] =
{
	[0] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x6a},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x00},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x15},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x2a},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
	},

	[1] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x6a},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x00},
		[LEFT]		= {.header = 0xb0, .arg1 = 0x01, .arg2 = 0x7f},
		[RIGHT]		= {.header = 0xb0, .arg1 = 0x01, .arg2 = 0x40},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|LEFT]	= {.header = 0xb0, .arg1 = 0x01, .arg2 = 0x00},
		[CENTER|RIGHT]	= {.header = 0xb0, .arg1 = 0x01, .arg2 = 0x00},
	},

	[2] =
	{
		[UP] 		= {.header = 0xb0, .arg1 = 0x0b, .arg2 = 0x7f},
		[DOWN]		= {.header = 0xb0, .arg1 = 0x0b, .arg2 = 0x40},
		[LEFT]		= {.header = 0xb0, .arg1 = 0x01, .arg2 = 0x7f},
		[RIGHT]		= {.header = 0xb0, .arg1 = 0x01, .arg2 = 0x40},

		[CENTER|UP]	= {.header = 0xb0, .arg1 = 0x0b, .arg2 = 0x00},
		[CENTER|DOWN]	= {.header = 0xb0, .arg1 = 0x0b, .arg2 = 0x00},
		[CENTER|LEFT]	= {.header = 0xb0, .arg1 = 0x01, .arg2 = 0x00},
		[CENTER|RIGHT]	= {.header = 0xb0, .arg1 = 0x01, .arg2 = 0x00},
	},

	[3] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x35},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x25},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x1a},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x10},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x00},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x00},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x00},
	},

	[4] =
	{
		[UP] 		= {.header = 0xb0, .arg1 = 0x4a, .arg2 = 0x7f},
		[DOWN]		= {.header = 0xb0, .arg1 = 0x0b, .arg2 = 0x7f},
		[LEFT]		= {.header = 0xb0, .arg1 = 0x01, .arg2 = 0x7f},
		[RIGHT]		= {.header = 0xb0, .arg1 = 0x5b, .arg2 = 0x7f},

		[CENTER|UP]	= {.header = 0xb0, .arg1 = 0x4a, .arg2 = 0x00},
		[CENTER|DOWN]	= {.header = 0xb0, .arg1 = 0x0b, .arg2 = 0x00},
		[CENTER|LEFT]	= {.header = 0xb0, .arg1 = 0x01, .arg2 = 0x00},
		[CENTER|RIGHT]	= {.header = 0xb0, .arg1 = 0x5b, .arg2 = 0x00},
	},

	[5] =
	{
		[UP] 		= {.header = 0xb0, .arg1 = 0x2d, .arg2 = 0x7f},
		[DOWN]		= {.header = 0xb0, .arg1 = 0x2e, .arg2 = 0x7f},
		[LEFT]		= {.header = 0xb0, .arg1 = 0x2f, .arg2 = 0x7f},
		[RIGHT]		= {.header = 0xb0, .arg1 = 0x30, .arg2 = 0x7f},

		[CENTER|UP]	= {.header = 0xb0, .arg1 = 0x2d, .arg2 = 0x00},
		[CENTER|DOWN]	= {.header = 0xb0, .arg1 = 0x2e, .arg2 = 0x00},
		[CENTER|LEFT]	= {.header = 0xb0, .arg1 = 0x2f, .arg2 = 0x00},
		[CENTER|RIGHT]	= {.header = 0xb0, .arg1 = 0x30, .arg2 = 0x00},
	},

	[6] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x55},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x50},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x30},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x2a},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
	},

	[7] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x15},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x1a},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x25},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x3a},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x15},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x1a},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
	},

	[8] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x1a},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x4a},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x15},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x2a},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x3a},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x25},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x00},
	},

	[9] =
	{
		[UP] 		= {.header = 0xb0, .arg1 = 0x07, .arg2 = 0x6e},
		[DOWN]		= {.header = 0xb0, .arg1 = 0x07, .arg2 = 0x78},
		[LEFT]		= {.header = 0xb0, .arg1 = 0x07, .arg2 = 0x64},
		[RIGHT]		= {.header = 0xb0, .arg1 = 0x07, .arg2 = 0x5a},

		[CENTER|UP]	= {.header = 0xb0, .arg1 = 0x07, .arg2 = 0x6e},
		[CENTER|DOWN]	= {.header = 0xb0, .arg1 = 0x07, .arg2 = 0x78},
		[CENTER|LEFT]	= {.header = 0xb0, .arg1 = 0x07, .arg2 = 0x64},
		[CENTER|RIGHT]	= {.header = 0xb0, .arg1 = 0x07, .arg2 = 0x5a},
	},

	[10] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x00},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x1a},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x0d},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x12},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x20},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x12},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x10},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x1a},
	},

	[11] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x10},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x25},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x35},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x0a},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x15},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x00},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x00},
	},

	[12] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x50},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x4a},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x3a},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x30},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
	},

	[13] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x50},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x45},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x35},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x30},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
	},

	[14] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x50},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x4a},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x55, .arg2 = 0x3a},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x2b, .arg2 = 0x35},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
	},

	[15] =
	{
		[UP] 		= {.header = 0xe0, .arg1 = 0x7f, .arg2 = 0x7f},
		[DOWN]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x60},
		[LEFT]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x20},
		[RIGHT]		= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x00},

		[CENTER|UP]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|DOWN]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|LEFT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
		[CENTER|RIGHT]	= {.header = 0xe0, .arg1 = 0x00, .arg2 = 0x40},
	},
};
