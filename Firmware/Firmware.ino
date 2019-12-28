#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <avr/pgmspace.h>
#include "fonts/terminus12.h"
#include "fonts/terminus24.h"

#include "images/github.h"
#include "images/pony.h"

#include "libraries/fontController.h"

#define displayWidth  128 // Ширина экрана в пикселях
#define displayHeight 64  // Высота экрана в пикселях

Adafruit_SSD1306 display(displayWidth, displayHeight, &Wire, 4); // Библиотека дисплея

void drawCopyright(){
	display.clearDisplay();
	//display.drawBitmap(48, 26, image_github, 16, 16, 1);
	display.drawBitmap(0, 6, image_pony, 32, 53, 1);
	FontManager::drawTextFormated(display, 32, 0, displayWidth, displayHeight, font_terminus12, (char*)"  Ponywka");
	display.display();
}

void setup() {
	Serial.begin(9600);
	// Инициализация дисплея
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // тут под ставить свой I2C адресс
		Serial.println(F("SSD1306 allocation failed"));
		for(;;);
	}

	drawCopyright();
}

void loop() {

}
