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

//	Датчики
#define pwmSolderPin	10
#define pwmFanPin		11
#define gerconPin		12
#define relayPin		13

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

uint16_t pwmSolder = 0;
uint8_t pwmFan = 0;
boolean isOn = false;

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

void menu0(){
	// Надпись OFF
	fntCtrl.setFont(font_terminus24);
	fntCtrl.drawTextFormated(0, 16, displayWidth, displayHeight - 16, CenterCenter, Left, "OFF");
	// Текущая температура
	outString = "";
	outString.concat("Current: ");
	outString.concat(String(thermocoupleTemperature));
	outString.concat((char)128);
	outString.concat("C");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, 0, displayWidth, 16, CenterCenter, Left, outString.c_str());
}

void menu1(){
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
	outString.concat((char)128);
	outString.concat("C");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, 0, displayWidth, 16, CenterCenter, Left, outString.c_str());
	// Скорость вентилятора
	drawFan(0, displayHeight - 16);
	outString = "";
	outString.concat("  ");
	outString.concat(String(map(pwmFan,0,255,0,100)));
	outString.concat("%");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, displayHeight - 16, displayWidth, displayHeight, LeftCenter, Left, outString.c_str());
	// Разогрев
	outString = "";
	outString.concat("Load:");
	outString.concat(String(map(pwmSolder,0,1023,0,100)));
	outString.concat("%");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, displayHeight - 16, displayWidth, displayHeight, RightCenter, Left, outString.c_str());
}

void menu1Change(){
	// Выбранная температура
	fntCtrl.setFont(font_terminus24);
	outString = "";
	outString.concat(String(currentTemperature));
	outString.concat((char)128);
	outString.concat("C");
	fntCtrl.drawTextFormated(0, 16, displayWidth, displayHeight - 16, CenterCenter, Left, outString.c_str());
	// Текущая температура
	outString = "";
	outString.concat("Current: ");
	outString.concat(String(thermocoupleTemperature));
	outString.concat((char)128);
	outString.concat("C");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, 0, displayWidth, 16, CenterCenter, Left, outString.c_str());
	// Скорость вентилятора
	drawFan(0, displayHeight - 16);
	outString = "";
	outString.concat("  ");
	outString.concat(String(map(pwmFan,0,255,0,100)));
	outString.concat("%");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, displayHeight - 16, displayWidth, displayHeight, LeftCenter, Left, outString.c_str());
	// Разогрев
	outString = "";
	outString.concat("Load:");
	outString.concat(String(map(pwmSolder,0,1023,0,100)));
	outString.concat("%");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, displayHeight - 16, displayWidth, displayHeight, RightCenter, Left, outString.c_str());
}

unsigned long TimeMenu1Change = 0;
void showTimeoutedMenu1Change(){
	TimeMenu1Change = millis() + 1000;
}

void menu2(){
	// Текущая скорость вентилятора
	fntCtrl.setFont(font_terminus24);
	outString = "";
	outString.concat(String(map(pwmFan,0,255,0,100)));
	outString.concat("%");
	fntCtrl.drawTextFormated(0, 16, displayWidth, displayHeight - 16, CenterCenter, Left, outString.c_str());
	// Текущая температура
	outString = "";
	outString.concat("Current: ");
	outString.concat(String(thermocoupleTemperature));
	outString.concat((char)128);
	outString.concat("C");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, 0, displayWidth, 16, CenterCenter, Left, outString.c_str());
	// Разогрев
	outString = "";
	outString.concat("Load:");
	outString.concat(String(map(pwmSolder,0,1023,0,100)));
	outString.concat("%");
	fntCtrl.setFont(font_terminus12);
	fntCtrl.drawTextFormated(0, displayHeight - 16, displayWidth, displayHeight, RightCenter, Left, outString.c_str());
}

unsigned long TimeMenu2 = 0;
void showTimeoutedMenu2(){
	TimeMenu2 = millis() + 3000;
}

void refreshDisplay(){
	display.clearDisplay();
	if(isOn){
		if(millis() < TimeMenu1Change){
			menu1Change();
		}else if(millis() < TimeMenu2){
			menu2();
		}else{
			menu1();
		}
	}else{
		menu0();
	}

	display.display();
}

#define PULSE_PIN_LEN 1
byte prevA = 0;
byte B = 0;
unsigned long failingTime = 0;
unsigned long pulseLen = 0;
void encoder() {
    byte A = digitalRead(3);
    if (prevA && !A)
    {
        B = digitalRead(4);
        failingTime = millis();
    }
    if (!prevA && A && failingTime) {
        pulseLen = millis() - failingTime;
        if ( pulseLen > PULSE_PIN_LEN) {
			// Обработка поворота
			if(isOn){
				if(millis() < TimeMenu2){
					showTimeoutedMenu2();
					if (B) {
						// Поворот по часовой
						pwmFan += 5;
					} else {
						// Поворот против часовой
						pwmFan -= 5;
					}
				}else{
					showTimeoutedMenu1Change();
					if (B) {
						// Поворот по часовой
						currentTemperature += 5;
					} else {
						// Поворот против часовой
						currentTemperature -= 5;
					}
				}
			}
        }
        failingTime = 0;
    }
    prevA = A;
}

unsigned long encoderButtonClickTime;
void encoder_button() {
    if (digitalRead(2)) {
		if(millis() - encoderButtonClickTime < 1000){
			// Короткое нажатиe
			showTimeoutedMenu2();
		}else{
			// Длинное нажатие
			isOn = !isOn;
		}
    }else{
		encoderButtonClickTime = millis();
	}
}

/*
	Основная программа
*/
void setup()
{
	// ШИМ паяльника
	// Пины D9 и D10 - 7.5 Гц 10bit
	TCCR1A = 0b00000011;
	TCCR1B = 0b00000101;

	// ШИМ вентилятора
	// Пины D3 и D11 - 8 кГц
	TCCR2B = 0b00000010;  // x8
	TCCR2A = 0b00000011;  // fast pwm

    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT);
    pinMode(4, INPUT);

	pinMode(pwmFanPin, OUTPUT);
	pinMode(pwmSolderPin, OUTPUT);
	pinMode(gerconPin, OUTPUT);

    attachInterrupt(0, encoder_button, CHANGE);
    attachInterrupt(1, encoder, CHANGE);

	Serial.begin(115200);
	// Инициализация дисплея
	if (!display.begin(SSD1306_SWITCHCAPVCC, displayAddres))
	{
		Serial.println(F("SSD1306 allocation failed"));
		for (;;)
			;
	}

	pwmFan = 255;
	currentTemperature = 0;

	PID.setLimits(0, 1023);
}

unsigned long thermocoupleOldTime, thermocoupleNewTime;
unsigned long logOldTime, logNewTime;
void loop()
{
	// Опрос термопары
	thermocoupleNewTime = millis() / thermocoupleTimeout;
	if (thermocoupleOldTime != thermocoupleNewTime)
	{
		thermocoupleTemperature = thermocouple.readCelsius();
		thermocoupleOldTime = thermocoupleNewTime;
	}

	if(isOn){
		PID.input = thermocoupleTemperature;
		PID.setpoint = currentTemperature;
		pwmSolder = PID.getResult();
		//	Исправление 100% заполнения ШИМ'а
		if(pwmSolder == 255) pwmSolder = 254;
	}else{
		pwmSolder = 0;
		if(thermocoupleTemperature > 25){
			pwmFan = 255;
		}else{
			pwmFan = 0;
		}
	}

	analogWrite(pwmFanPin, pwmFan);
	analogWrite(pwmSolderPin, pwmSolder);

	refreshDisplay();

	#ifdef DEBUG
		logOldTime = logNewTime;
		logNewTime = millis();
		Serial.println(logNewTime - logOldTime);
	#endif
}
