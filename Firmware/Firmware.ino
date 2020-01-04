#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <avr/pgmspace.h>
#include "fonts/terminus12.h"
#include "fonts/terminus24.h"

#include "images/github.h"
#include "images/pony.h"

#include "libraries/fontController.h"

/*
*	Дисплей
*/
#define displayWidth 128   // Ширина [пиксели]
#define displayHeight 64   // Высота [пиксели]
#define displayAddres 0x3D // Адрес дисплея

Adafruit_SSD1306 display(displayWidth, displayHeight, &Wire, 4); // Библиотека дисплея
FontController fntCtrl(display);

int n = 0;

void drawCopyright()
{
	fntCtrl.setFont(font_terminus12);
	display.clearDisplay();
	//display.drawBitmap(48, 26, image_github, 16, 16, 1);
	display.drawBitmap(0, 6, image_pony, 32, 53, 1);
	fntCtrl.drawTextFormated(32, 0, displayWidth-1, displayHeight-1, CenterCenter, Center, "GitHub:\n@Ponywka");
	display.display();
}

void setup()
{
	Serial.begin(115200);
	// Инициализация дисплея
	if (!display.begin(SSD1306_SWITCHCAPVCC, displayAddres))
	{
		Serial.println(F("SSD1306 allocation failed"));
		for (;;)
			;
	}

	//fntCtrl.setFont(font_terminus12);
	/*display.clearDisplay();
	fntCtrl.drawTextFormated(0, 0, 127, 63, LeftTop, Right, "aaaaa\nbbb\nc");
	display.display();*/
	drawCopyright();
	delay(3000);
	//fntCtrl.setFont(font_terminus24);
}

void loop()
{
	display.clearDisplay();
	fntCtrl.drawTextFormated(0, 0, displayWidth, displayHeight, CenterCenter, Left, (char *)(("Frame: " + String(n)).c_str()));
	display.display();
	n++;
}
