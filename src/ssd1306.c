#include "ssd1306.h"

uint8_t buffer[BUFFER_SIZE];

/*
	The font stores information about itself in a header,
	the first 6 items of the array. This includes the font width,
	the height, the start position of characters, etc.
*/
#define FONTWIDTH font5x7[0]
#define FONTHEIGHT font5x7[1]
#define FONTSTART 6
#define SPACING 1

// debugging
#include <avr/io.h>

void ledOn() {
	PORTB &= ~(1<<0);
}
void ledOff() {
	PORTB |= (1<<0);
}

void ledInit() {
	DDRB = 0xFF;
}

// SSD1306
void initScreen(char address) {
	dimState = 0;
	sleepState = 0;
	previousTime = time(NULL);
	oled_address = address;
	currentRow = 0;
	currentCol = 0;
	/*
		Initialise I2C.
	*/
	i2cInit();
	i2cStart();
	i2cSendAddress(address, 1);
	/*
		Initialise screen.
	*/
	i2cSendByte(0x00);
	i2cSendByte(displayOff);
	i2cSendData(2, setOscFreq, 0x80);
	i2cSendData(2, setMultiplexRatio, 0x3F);
	i2cSendData(2, setDisplayOffset, 0x00);
	i2cSendByte(setStartLine | 0x00);
	i2cSendData(2, setChargePump, 0x14);
	i2cSendData(2, setMemAddrMode, 0x00);
	i2cSendByte(setSegRemap | 0x01);
	i2cSendByte(setComScanDir);
	i2cSendData(3, setColAddr, 0x00, (COLS - 1));
	i2cSendData(2, setComPins, 0x12);
	i2cSendData(2, setContrast, 0xFF);
	i2cSendData(2, setPreCharge, 0xF1);
	i2cSendData(2, setVCOMHDetect, 0x40);
	i2cSendByte(displayAllOnRAM);
	i2cSendByte(normalDisplay);
	i2cSendByte(deactivateScroll);
	i2cSendByte(displayOn);
	i2cStop();
}

void clearBuffer() {
	for (int i = 0; i < BUFFER_SIZE; i++) {
		buffer[i] = 0;
	}
}

void clearScreen() {
	clearBuffer();
	updateScreen();
}

void updateScreen() {
	i2cStart();
	i2cSendAddress(oled_address, 1);
	i2cSendByte(0x40);
	for (int i = 0; i < BUFFER_SIZE; i++) {
		i2cSendByte(buffer[i]);
	}
	i2cStop();
}

void drawPixel(int x, int y) {
	/*
		To draw pixels:
		We take a co-ordinate (X,Y).
		X co-ordinates represent columns.
		Y co-ordinates represent rows.
		A byte is 8 rows high, and 1 column wide.
		A page on the screen is 1 byte high, and 128 columns wide.
		We bit shift 1 into the correct position in a byte,
		and OR it against the byte in the buffer where (X,Y) resides.
	*/
	int byteNumber, pageNumber;
	pageNumber = y / 8;
	byteNumber = (COLS * pageNumber) + x;
	// Check if (X,Y) is within the boundaries of our screen.
	if ((x >= 0 && x < COLS) && (y >= 0 && y < ROWS)) {
		buffer[byteNumber] |= 0x80 >> (((pageNumber * 8) + 7) - y);
	}
	// buffer[byteNumber] |= 0x80 >> (((pageNumber * 8) + 7) - y);
}

void drawChar(int charToDraw, int xOffset, int yOffset) {
	/*
		To print characters from the font:
		We loop through the bytes in the font array
		and inspect each bit in those bytes.
		Each byte represents one column in a character.
		Each bit in a byte represents a row in a character.
		We loop through the bits in each byte, bit-shifting them to the end of the byte,
		and AND them with 1, which can produce only 1 or 0.
		By keeping track of the byte and bit indicies in the loop, we know X and Y values.
		If the bit is 1, we draw that pixel.
	*/
	int startByteIndex = FONTSTART + (FONTWIDTH * charToDraw);
	for (int currentByteIndex = 0; currentByteIndex < FONTWIDTH; currentByteIndex++) {
		char currentByte = font5x7[startByteIndex + currentByteIndex];
		for (int bitIndex = 7; bitIndex >= 0; bitIndex--) {
			char currentBit = (currentByte >> bitIndex) & 1;
			if (currentBit) {
				drawPixel((currentByteIndex + xOffset), (bitIndex + yOffset));
			}
		}
	}
}

void drawString(char* stringToDraw) {
	/*
		To print strings of text:
		We calculate the maximum number of characters
		that we can display on screen without any clipping,
		then we loop through the characters in the string.
		Starting at (0,0), we draw characters to the screen,
		incrementing the column we are on each time.
		If the character we are going to draw will clip
		the Y boundary of the screen, we increment the row,
		reset the column to 0, and continue drawing.
	*/
	int maxCharacters = COLS / (FONTWIDTH + SPACING);
	int length = strlen(stringToDraw);

	for (int i = 0; i < length; i++) {
		if (currentCol >= maxCharacters) {
			currentRow++;
			currentCol = 0;
		}
		switch (stringToDraw[i]) {
			case '\n':
				currentRow++;
				currentCol = 0;
				break;
			case ' ':
				if (currentCol == 0) {
					break;
				}
			default:
				drawChar(stringToDraw[i], (currentCol * (FONTWIDTH + SPACING)), (FONTHEIGHT * currentRow));
				currentCol++;
		}
	}
}

int getPixelState(int x, int y) {
	/*
		To get the state of a pixel:
		We take the byte that the pixel is stored in,
		bit shift to the end of the byte, then
		AND it with 1, to return 0 or 1.
	*/
	int byteNumber, pageNumber;
	pageNumber = y / 8;
	byteNumber = (COLS * pageNumber) + x;
	return ((buffer[byteNumber] >> (((pageNumber * 8) + 7) - y) & 1));
}

void dimScreen() {
	i2cStart();
	i2cSendAddress(oled_address, 1);
	i2cSendByte(0x00);
	for (int i = 0; i < 2; i++) {
		for (int c = 255; c >= 0; c--) {
			i2cSendData(2, setContrast, c);
			_delay_ms(DIMMING_DELAY);
		}
		if (i == 0) {
			i2cSendData(2, setVCOMHDetect, 0);
		}
	}
	i2cStop();
	dimState = 1;
}

void sleepScreen() {
	if (!dimState) {
		dimScreen();
		dimState = 1;
	}
	i2cStart();
	i2cSendAddress(oled_address, 1);
	i2cSendByte(0x00);
	i2cSendByte(displayOff);
	i2cStop();
	sleepState = 1;
}

void wakeScreen() {
	i2cStart();
	i2cSendAddress(oled_address, 1);
	i2cSendByte(0x00);
	if (sleepState) {
		i2cSendByte(displayOn);
		sleepState = 0;
	}
	if (dimState) {
		for (int i = 0; i < 2; i++) {
			for (int c = 0; c < 256; c++) {
				i2cSendData(2, setContrast, c);
			}
			if (i == 0) {
				i2cSendData(2, setVCOMHDetect, 0x40);
			}
		}
		dimState = 0;
	}
	i2cStop();
}

// debugging
#ifdef unix
void printBuffer() {
	printf("┏");
	for (int i = 0; i < COLS; i++) {
		printf("━━");
	}
	printf("┓\n");

	for (int y = 0; y < ROWS; y++) {
		printf("┃");
		for (int x = 0; x < COLS; x++) {
			if (getPixelState(x, y)) {
				printf("⬛");
			}
			else {
				printf("⬜");
			}
		}
		printf("┃\n");
	}

	printf("┗");
	for (int i = 0; i < COLS; i++) {
		printf("━━");
	}
	printf("┛\n");
}
#endif
