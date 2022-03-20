#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <GyverPID.h>
#include "libraries/fontController.h"

#include "fonts/terminus12.h"
#include "fonts/terminus24.h"
#include "images/fan1.h"
#include "images/fan2.h"
#include "images/fan3.h"

/*
	Настройки
*/

//	Служебные
#define DEBUG

//	Дисплей
#define displayWidth 128   // Ширина [пиксели]
#define displayHeight 64   // Высота [пиксели]
#define displayResetPin -1 // Пин перезагрузки дисплея (-1 для I2C)
#define displayAddres 0x3D // Адрес дисплея

//	Термопара
#define thermocoupleSCK 5
#define thermocoupleCS 6
#define thermocoupleMISO 7
#define thermocoupleTimeout 250
#define thermocoupleProteus

//	Настройки PID
#define PID_Kp	20	// пропорциональный коэффициент, выходная величина будет увеличиваться пропорционально разнице входного сигнала и установки
#define PID_Ki	40	// коэффициент интегрирующей составляющей, отвечает за накапливающуюся ошибку, позволяет сгладить пульсации и нивелировать маленькую ошибку
#define PID_Kd	4	// коэффициент дифференциальной составляющей, отвечает за скорость изменения величины, позволяет уменьшить раскачку системы

int16_t thermocoupleTemperature;
int16_t currentTemperature;

#ifdef thermocoupleProteus
	#include <Adafruit_MAX31855.h>
	Adafruit_MAX31855 thermocouple(thermocoupleSCK, thermocoupleCS, thermocoupleMISO);
#else
	#include <max6675.h>
	MAX6675 thermocouple(thermocoupleSCK, thermocoupleCS, thermocoupleMISO);
#endif

Adafruit_SSD1306 display(displayWidth, displayHeight, &Wire, displayResetPin);
FontController fntCtrl(display);
GyverPID PID(PID_Kp, PID_Ki, PID_Kd);

uint8_t pwdSolder = 0;
uint8_t pwdFan = 0;

/*
	Вспомогательные функции
*/
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

String outString = "";
void refreshDisplay(){
	display.clearDisplay();

	// Текущая температура
	fntCtrl.setFont(font_terminus24);
	outString = "";
	outString.concat(String(thermocoupleTemperature));
	outString.concat((char)128);
	outString.concat("C");
	fntCtrl.drawTextFormated(0, 16, displayWidth, displayHeight - 16, CenterCenter, Left, outString.c_str());
	// Выбранная температура
	outString = "";
	outString.concat("Selected: ");
	outString.concat(String(currentTemperature));
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, 0, displayWidth, 16, CenterCenter, Left, outString.c_str());
	// Скорость вентилятора
	drawFan(0, displayHeight - 16);
	outString = "";
	outString.concat("  ");
	outString.concat(String(map(pwdFan,0,255,0,100)));
	outString.concat("%");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, displayHeight - 16, displayWidth, displayHeight, LeftCenter, Left, outString.c_str());
	// Разогрев
	outString = "";
	outString.concat("Load:");
	outString.concat(String(map(pwdSolder,0,255,0,100)));
	outString.concat("%");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, displayHeight - 16, displayWidth, displayHeight, RightCenter, Left, outString.c_str());

	display.display();
}

/*
	Основная программа
*/
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

	pwdFan = 255;
	currentTemperature = 100;
}

unsigned long thermocoupleOldTime, thermocoupleNewTime;
unsigned long logOldTime, logNewTime;
void loop()
{
	thermocoupleNewTime = millis() / thermocoupleTimeout;
	if (thermocoupleOldTime != thermocoupleNewTime)
	{
		thermocoupleTemperature = thermocouple.readCelsius();
		thermocoupleOldTime = thermocoupleNewTime;
	}
	
	PID.input = thermocoupleTemperature;
	PID.setpoint = currentTemperature;
	pwdSolder = PID.getResult();
	refreshDisplay();

	#ifdef DEBUG
		logOldTime = logNewTime;
		logNewTime = millis();
		Serial.println(logNewTime - logOldTime);
	#endif
}
