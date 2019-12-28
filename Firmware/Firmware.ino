#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <avr/pgmspace.h>
#include "fonts/terminus12.h"
#include "fonts/terminus24.h"

#include "images/github.h"
#include "images/pony.h"

#include "libraries/fontController.h"

const int DisplayY = 64; // пикселей по вертикале
const int Displayx = 128; // пикселей по горизонтали

Adafruit_SSD1306 display(Displayx, DisplayY, &Wire, 4); // добавляем новый дисплей

/*
	Font format:
	0: width (16px)
	1: height (24px)
	2: bytesByLine
	3: charArrayLenght
	4-*: chars
	*-*: bitmap
*/

void setup() {
	Serial.begin(9600);
	// Инициализация дисплея
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
		Serial.println(F("SSD1306 allocation failed"));
		for(;;);
	}

	display.clearDisplay();
	display.drawBitmap(48, 26, image_github, 16, 16, 1);
	display.drawBitmap(0, 6, image_pony, 32, 53, 1);
	FontManager::drawText(display, 48, 26, font_terminus12, (char*)"  Ponywka");
	display.display();
}

void loop() {

}
