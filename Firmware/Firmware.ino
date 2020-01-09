#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <avr/pgmspace.h>
#include "fonts/terminus12.h"
#include "fonts/terminus24.h"

#include "images/github.h"
#include "images/pony.h"

#include "images/fan1.h"
#include "images/fan2.h"
#include "images/fan3.h"

#include "libraries/fontController.h"

/*
*	Дисплей
*/
#define displayWidth 128   // Ширина [пиксели]
#define displayHeight 64   // Высота [пиксели]
#define displayAddres 0x3D // Адрес дисплея

/*
*	Термопара
*/
#define tempSCK 5
#define tempCS 6
#define tempSO 7
#define tempTimeout 250
#define tempProteus

int16_t temperature;
int16_t temperatureSet;
uint16_t tempLastMillis;
uint16_t tempNewMillis;

#ifdef tempProteus
#include <Adafruit_MAX31855.h>
Adafruit_MAX31855 temp(tempSCK, tempCS, tempSO);
#else
#include <max6675.h>
MAX6675 temp(tempSCK, tempCS, tempSO);
#endif

Adafruit_SSD1306 display(displayWidth, displayHeight, &Wire, 4); // Библиотека дисплея
FontController fntCtrl(display);

int n = 0;

void drawCopyright()
{
	fntCtrl.setFont(font_terminus12);
	display.clearDisplay();
	//display.drawBitmap(48, 26, image_github, 16, 16, 1);
	display.drawBitmap(0, 6, image_pony, 32, 53, 1);
	fntCtrl.drawTextFormated(32, 0, displayWidth - 1, displayHeight - 1, CenterCenter, Center, (char *)"GitHub:\n@Ponywka");
	display.display();
}

uint8_t animstate;
void drawFan(uint16_t x, uint16_t y)
{
	animstate = (millis() / 100) % 3;
	if (animstate == 0)
		display.drawBitmap(x, y, image_fan1, 16, 16, 1);
	if (animstate == 1)
		display.drawBitmap(x, y, image_fan2, 16, 16, 1);
	if (animstate == 2)
		display.drawBitmap(x, y, image_fan3, 16, 16, 1);
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
	temperatureSet = 100;
}

void loop()
{
	tempNewMillis = millis() / tempTimeout;
	if (tempLastMillis != tempNewMillis)
	{
		temperature = temp.readCelsius();
		tempLastMillis = tempNewMillis;
	}

	display.clearDisplay();
	// Текущая температура
	fntCtrl.setFont(font_terminus24);
	fntCtrl.drawTextFormated(0, 16, displayWidth, displayHeight - 16, CenterCenter, Left, (char *)((String(temperature) + (char)128 + "C").c_str()));
	// Остальное
	fntCtrl.setFont(font_terminus12);
	// Выбранная температура
	fntCtrl.drawTextFormated(0, 0, displayWidth, 16, CenterCenter, Left, (char *)(("Selected: " + String(temperatureSet) + (char)128 + "C").c_str()));
	// Скорость вентилятора
	drawFan(0, displayHeight - 16);
	fntCtrl.drawTextFormated(0, displayHeight - 16, displayWidth, displayHeight, LeftCenter, Left, (char *)(("  " + String(temperatureSet) + "%").c_str()));
	// Разогрев
	fntCtrl.drawTextFormated(0, displayHeight - 16, displayWidth, displayHeight, RightCenter, Left, (char *)(("Load:" + String(temperatureSet) + "%").c_str()));

	display.display();
}
