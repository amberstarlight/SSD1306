#ifndef SSD1306_H
#define SSD1306_H

#include <string.h>
#include <util/delay.h>
#include <time.h>
#include "i2c.h"

#include "oled_commands.h"
#include "font5x7.h"

#define COLS 128
#define ROWS 64
#define BUFFER_SIZE COLS * (ROWS / 8)

#define DIMMING_DELAY 5

extern uint8_t buffer[BUFFER_SIZE];
int currentRow, currentCol;
char dimState, sleepState;

#ifdef unix
#include <stdio.h>
void printBuffer();
#endif

// debugging
void ledInit();
void ledOn();
void ledOff();

// SSD1306
void initScreen(char address);
void clearBuffer();
void clearScreen();
void updateScreen();
void drawPixel(int x, int y);
void drawChar(int charToDraw, int xOffset, int yOffset);
void drawString(char* stringToDraw);
int  getPixelState(int x, int y);
void dimScreen();
void sleepScreen();
void wakeScreen();

unsigned char oled_address;
time_t previousTime;

#endif
