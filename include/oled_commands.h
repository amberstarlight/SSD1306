#ifndef OLED_COMMANDS_H
#define OLED_COMMANDS_H

// fundamental commands
#define setContrast						0x81
#define displayAllOnRAM				0xA4
#define displayAllOn					0xA5
#define normalDisplay					0xA6
#define invertDisplay					0xA7
#define displayOff 						0xAE
#define displayOn							0xAF

// scrolling commands
#define rightHorizontalScroll 0x26
#define leftHorizontalScroll	0x27
#define rightHorVerScroll			0x29
#define leftHorVerScroll			0x2A
#define deactivateScroll			0x2E
#define activateScroll				0x2F
#define setScrollArea					0xA3

// addressing commands
#define setLowCol							0x00
#define setHighCol						0x10
#define setMemAddrMode				0x20
#define setColAddr						0x21
#define setPageAddr						0x22 /* only for horizontal or vertical addressing mode */
#define setPageStart					0xB0 /* only for page addressing mode */

// hardware configuration commands
#define setStartLine 					0x40
#define setSegRemap						0xA0
#define setMultiplexRatio			0xA8
#define setComScanDir					0xC8
#define setDisplayOffset			0xD3
#define setComPins						0xDA

// timing & driving commands
#define setOscFreq						0xD5
#define setPreCharge 					0xD9
#define setChargePump					0x8D
#define setVCOMHDetect				0xDB
#define NOP										0xE3

#endif
