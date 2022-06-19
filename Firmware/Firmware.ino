/*
	Настройки
*/

//	[Служебные]
// #define DEBUG
// #define PROTEUS_MODE

//	[Датчики]
#define gerconPin		12
#define relayPin		13

//	[Дисплей]
// Требуется раскомментировать один из дисплеев
// Можно использовать и сразу два, если конечно-же будет место :)

// OLED дисплей
#define displaySSD1306_Enabled
#ifdef displaySSD1306_Enabled
	#define displaySSD1306Width 128   // Ширина [пиксели]
	#define displaySSD1306Height 64   // Высота [пиксели]
	#define displaySSD1306ResetPin -1 // Пин перезагрузки дисплея (-1 для I2C)
	#define displaySSD1306Address 0x3C // Адрес дисплея
#endif

// 7-ми сегментный дисплей
// #define displayTM1637_Enabled
#ifdef displayTM1637_Enabled
	#define displaySSD1306CLKPin 14 // A2
	#define displaySSD1306DIOPin 15 // A3
	#define displaySSD1306RefreshRate	100;
#endif

//	[Термопара]
#define thermocoupleSCK 5
#define thermocoupleCS 6
#define thermocoupleMISO 7
#define thermocoupleTimeout 250

//	[Настройки PID] // Ku = 160
#define PID_Kp	96
#define PID_Ki	12
#define PID_Kd	192
#define PID_Td	500

//	[Настройки PWM]
// Паяльник (0-1023)
#define pwmSolderMin 0
#define pwmSolderMax 500
// Вентилятор (0-255)
#define pwmFanMin 128
#define pwmFanMax 255

//	[Коррекция температуры]
#define correctionFormula thermocoupleTemperature * 17 / 20 + 20

//	[Остальные настройки]
#define temperatureEmergencyStop 450
#define temperatureWhenOffFanOff 75
#define temperatureWhenOffFanOn  100
#define encoderLongButtonTime    500

/*
	Основная программа
*/
#include <GyverPID.h>
#include "libraries/gercon.h"
int fanSpeed = 100;
long thermocoupleTemperature;
int currentTemperature = 100;

#ifndef PROTEUS_MODE
	#include <max6675.h>
	MAX6675 thermocouple(thermocoupleSCK, thermocoupleCS, thermocoupleMISO);
#else
	#include <Adafruit_MAX31855.h>
	Adafruit_MAX31855 thermocouple(thermocoupleSCK, thermocoupleCS, thermocoupleMISO);
#endif

#ifdef displaySSD1306_Enabled
	#include <Adafruit_SSD1306.h>
	#include "libraries/fontController.h"
	#include "fonts/terminus12.h"
	#include "fonts/terminus24.h"
	#include "images/fan1.h"
	#include "images/fan2.h"
	#include "images/fan3.h"

	Adafruit_SSD1306 displaySSD1306(displaySSD1306Width, displaySSD1306Height, &Wire, displaySSD1306ResetPin);
	FontController fntCtrl(displaySSD1306);
#endif

#ifdef displayTM1637_Enabled
	#include <GyverTM1637.h>
	byte displayTM1637Buffer[4] = {0, 0, 0, 0};
	GyverTM1637 displayTM1637(displaySSD1306CLKPin, displaySSD1306DIOPin);
#endif
GyverPID PID(PID_Kp, PID_Ki, PID_Kd);
Gercon gercon(gerconPin);

uint16_t pwmSolder = 0;
uint8_t pwmFan = 0;
boolean isOn = false;
boolean offFanComplete = false;
void offHeat(){
	isOn = false;
	offFanComplete = false;
}

#ifdef displaySSD1306_Enabled
	uint8_t animstate;
	void drawFan(uint16_t x, uint16_t y)
	{
		animstate = (millis() / 100) % 3;
		if (animstate == 0)
			displaySSD1306.drawBitmap(x, y, image_fan1, 16, 16, 1);
		if (animstate == 1)
			displaySSD1306.drawBitmap(x, y, image_fan2, 16, 16, 1);
		if (animstate == 2)
			displaySSD1306.drawBitmap(x, y, image_fan3, 16, 16, 1);
	}
#endif

#ifdef displayTM1637_Enabled
	void displayTM1637_writeInt(int value){
		value = abs(value);
		byte digit;
		byte isWritedFirstDigit = false;
		for(int8_t pos = 3; pos >= 0; pos--){
			if(value != 0 || !isWritedFirstDigit){
				isWritedFirstDigit = true;
				digit = value % 10;
				value = value / 10;

				switch(digit){
					case 0:
						displayTM1637Buffer[pos] = _0;
						break;
					case 1:
						displayTM1637Buffer[pos] = _1;
						break;
					case 2:
						displayTM1637Buffer[pos] = _2;
						break;
					case 3:
						displayTM1637Buffer[pos] = _3;
						break;
					case 4:
						displayTM1637Buffer[pos] = _4;
						break;
					case 5:
						displayTM1637Buffer[pos] = _5;
						break;
					case 6:
						displayTM1637Buffer[pos] = _6;
						break;
					case 7:
						displayTM1637Buffer[pos] = _7;
						break;
					case 8:
						displayTM1637Buffer[pos] = _8;
						break;
					case 9:
						displayTM1637Buffer[pos] = _9;
						break;
				}
			}
		}
	}
#endif

#define MENU
#ifdef MENU
	#ifdef displaySSD1306_Enabled
		String outString = "";
		#define STR_START outString = ""; outString.concat(
		#define STR_CON ); outString.concat(
		#define STR_END );
	#endif

	void menu0(){
		#ifdef displayTM1637_Enabled
			displayTM1637Buffer[0] = 0;
			displayTM1637Buffer[1] = _O;
			displayTM1637Buffer[2] = _F;
			displayTM1637Buffer[3] = _F;
		#endif
		#ifdef displaySSD1306_Enabled
			// Надпись OFF
			fntCtrl.setFont(font_terminus24);
			fntCtrl.drawTextFormated(0, 16, displaySSD1306Width, displaySSD1306Height - 16, CenterCenter, Left, "OFF");
			// Текущая температура
			STR_START "Current: " STR_CON String((int)thermocoupleTemperature) STR_CON (char)128 STR_CON "C" STR_END
			fntCtrl.setFont(font_terminus12);
			fntCtrl.drawTextFormated(0, 0, displaySSD1306Width, 16, CenterCenter, Left, outString.c_str());
		#endif
	}

	void menu1(){
		#ifdef displayTM1637_Enabled
			displayTM1637_writeInt(thermocoupleTemperature);
		#endif
		#ifdef displaySSD1306_Enabled
			// Текущая температура
			STR_START String((int)thermocoupleTemperature) STR_CON (char)128 STR_CON "C" STR_END
			fntCtrl.setFont(font_terminus24);
			fntCtrl.drawTextFormated(0, 16, displaySSD1306Width, displaySSD1306Height - 16, CenterCenter, Left, outString.c_str());
			// Выбранная температура
			STR_START "Selected: " STR_CON String(currentTemperature) STR_CON (char)128 STR_CON "C" STR_END
			fntCtrl.setFont(font_terminus12);
			fntCtrl.drawTextFormated(0, 0, displaySSD1306Width, 16, CenterCenter, Left, outString.c_str());
			// Скорость вентилятора
			drawFan(0, displaySSD1306Height - 16);
			STR_START "  " STR_CON String(fanSpeed) STR_CON "%" STR_END
			fntCtrl.setFont(font_terminus12);
			fntCtrl.drawTextFormated(0, displaySSD1306Height - 16, displaySSD1306Width, displaySSD1306Height, LeftCenter, Left, outString.c_str());
			// Разогрев
			STR_START "Load:" STR_CON String((int)map(pwmSolder,0,1023,0,100)) STR_CON "%" STR_END
			fntCtrl.setFont(font_terminus12);
			fntCtrl.drawTextFormated(0, displaySSD1306Height - 16, displaySSD1306Width, displaySSD1306Height, RightCenter, Left, outString.c_str());
		#endif
	}

	void menu1Change(){
		#ifdef displayTM1637_Enabled
			displayTM1637_writeInt(currentTemperature);
			displayTM1637Buffer[0] = _C;
		#endif
		#ifdef displaySSD1306_Enabled
			// Выбранная температура
			fntCtrl.setFont(font_terminus24);
			STR_START String(currentTemperature) STR_CON (char)128 STR_CON "C" STR_END
			fntCtrl.drawTextFormated(0, 16, displaySSD1306Width, displaySSD1306Height - 16, CenterCenter, Left, outString.c_str());
			// Текущая температура
			STR_START "Current: " STR_CON String((int)thermocoupleTemperature) STR_CON (char)128 STR_CON "C" STR_END
			fntCtrl.setFont(font_terminus12);
			fntCtrl.drawTextFormated(0, 0, displaySSD1306Width, 16, CenterCenter, Left, outString.c_str());
			// Скорость вентилятора
			drawFan(0, displaySSD1306Height - 16);
			STR_START "  " STR_CON String(fanSpeed) STR_CON "%" STR_END
			fntCtrl.setFont(font_terminus12);
			fntCtrl.drawTextFormated(0, displaySSD1306Height - 16, displaySSD1306Width, displaySSD1306Height, LeftCenter, Left, outString.c_str());
			// Разогрев
			STR_START "Load:" STR_CON String((int)map(pwmSolder,0,1023,0,100)) STR_CON "%" STR_END
			fntCtrl.setFont(font_terminus12);
			fntCtrl.drawTextFormated(0, displaySSD1306Height - 16, displaySSD1306Width, displaySSD1306Height, RightCenter, Left, outString.c_str());
		#endif
	}

	unsigned long TimeMenu1Change = 0;
	void showTimeoutedMenu1Change(){
		TimeMenu1Change = millis() + 500;
	};

	void menu2(){
		#ifdef displayTM1637_Enabled
			displayTM1637_writeInt(fanSpeed);
			displayTM1637Buffer[0] = _F;
		#endif
		#ifdef displaySSD1306_Enabled
			// Текущая скорость вентилятора
			fntCtrl.setFont(font_terminus24);
			STR_START String(fanSpeed) STR_CON "%" STR_END
			fntCtrl.drawTextFormated(0, 16, displaySSD1306Width, displaySSD1306Height - 16, CenterCenter, Left, outString.c_str());
			// Текущая температура
			STR_START "Current: " STR_CON String((int)thermocoupleTemperature) STR_CON (char)128 STR_CON "C" STR_END
			fntCtrl.setFont(font_terminus12);
			fntCtrl.drawTextFormated(0, 0, displaySSD1306Width, 16, CenterCenter, Left, outString.c_str());
			// Разогрев
			STR_START "Load:" STR_CON String((int)map(pwmSolder,0,1023,0,100)) STR_CON "%" STR_END
			fntCtrl.setFont(font_terminus12);
			fntCtrl.drawTextFormated(0, displaySSD1306Height - 16, displaySSD1306Width, displaySSD1306Height, RightCenter, Left, outString.c_str());
		#endif
	}

	unsigned long TimeMenu2 = 0;
	void showTimeoutedMenu2(){
		TimeMenu2 = millis() + 1500;
	}

	#ifdef displayTM1637_Enabled
		unsigned long displayTM1637NextUpdate = 0;
	#endif
	void refreshDisplay(){
		#ifdef displayTM1637_Enabled
			displayTM1637Buffer[0] = 0;
			displayTM1637Buffer[1] = 0;
			displayTM1637Buffer[2] = 0;
			displayTM1637Buffer[3] = 0;
		#endif
		#ifdef displaySSD1306_Enabled
			displaySSD1306.clearDisplay();
		#endif

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

		#ifdef displayTM1637_Enabled
			if(displayTM1637NextUpdate < millis()){
				displayTM1637.displayByte(displayTM1637Buffer);
				displayTM1637NextUpdate = millis() + displaySSD1306RefreshRate;
			}
		#endif
		#ifdef displaySSD1306_Enabled
			displaySSD1306.display();
		#endif
	}
#endif

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
				int *valAddress;
				int step;
				int min;
				int max;
				if(millis() < TimeMenu2){
					showTimeoutedMenu2();
					valAddress = &fanSpeed;
					step = 5;
					min = 0;
					max = 100;
				}else{
					showTimeoutedMenu1Change();
					valAddress = &currentTemperature;
					step = 10;
					min = 0;
					max = 400;
				}

				if (B) {
					// Поворот по часовой
					if(*valAddress + step <= max){
						*valAddress += step;
					}
				} else {
					// Поворот против часовой
					if(*valAddress - step >= min){
						*valAddress -= step;
					}
				}
			}
        }
        failingTime = 0;
    }
    prevA = A;
}

// Короткое нажатиe
void encoder_button_shortClick() {
	showTimeoutedMenu2();
}

// Длинное нажатие
void encoder_button_longClick() {
	if(isOn){
		offHeat();
	}else{
		isOn = true;
	}
}

unsigned long encoderButtonClickTime;
void encoder_button() {
    if (digitalRead(2)) {
		if(millis() - encoderButtonClickTime < encoderLongButtonTime){
			encoderButtonClickTime = 0;
			encoder_button_shortClick();
		}
    }else{
		encoderButtonClickTime = millis();
	}
}

unsigned long frametimeOldTime, frametimeNewTime, frametime;
#ifdef DEBUG
	void debugPrintHeader(){
		Serial.print("frametime, ");
		Serial.print("pwmSolder, ");
		Serial.print("pwmFan, ");
		Serial.print("thermocoupleTemperature, ");
		Serial.println("currentTemperature");
	}

	#define SERIAL_PRINT Serial.print(
	#define SERIAL_NEXT ); Serial.print(" "); Serial.print(
	#define SERIAL_END ); Serial.println("");

	void debugPrintData(){
		SERIAL_PRINT frametime
		SERIAL_NEXT pwmSolder
		SERIAL_NEXT pwmFan
		SERIAL_NEXT thermocoupleTemperature
		SERIAL_NEXT currentTemperature
		SERIAL_END
	}
#endif

bool gerconActivated = false;
void setup()
{
	// ШИМ паяльника | D9 и D10 - 7.5 Гц 10bit
	TCCR1A = 0b00000011;
	TCCR1B = 0b00000101;

	// ШИМ вентилятора | D3 и D11 - 8 кГц 8bit
	TCCR2B = 0b00000010;
	TCCR2A = 0b00000011;
	
	// Настройка Hard-code пинов I/O
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT);
    pinMode(4, INPUT);
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);

	// Настройка переменных пинов I/O
	pinMode(relayPin, OUTPUT);

	// Апаратные прерывания для энкодера
    attachInterrupt(0, encoder_button, CHANGE);
    attachInterrupt(1, encoder, CHANGE);

	#ifdef DEBUG
		Serial.begin(9600);
		debugPrintHeader();	
	#endif

	// Инициализация дисплея
	#ifdef displayTM1637_Enabled
		displayTM1637.clear();
		displayTM1637.brightness(7);
	#endif
	#ifdef displaySSD1306_Enabled
		#ifdef PROTEUS_MODE
			#define displaySSD1306Address 0x3D
		#endif
		if (!displaySSD1306.begin(SSD1306_SWITCHCAPVCC, displaySSD1306Address))
		{
			#ifdef DEBUG
				Serial.println(F("SSD1306 allocation failed"));
			#endif
			for (;;)
				;
		}
	#endif

	pwmFan = 255;
	currentTemperature = 250;

	PID.setLimits(pwmSolderMin, pwmSolderMax);
	PID.setDt(PID_Td);

	gerconActivated = gercon.getState();
}

unsigned long thermocoupleOldTime, thermocoupleNewTime;
unsigned long pidOldTime, pidNewTime;
bool gerconLast = false;
bool gerconCurrent = false;
void loop()
{
    // Encoder
    // TODO: Remove all spaghetti code
    // IDK why, but I can't use attachInterrupt inside class without static method
	if (encoderButtonClickTime != 0 && millis() - encoderButtonClickTime >= encoderLongButtonTime){
		encoderButtonClickTime = 0;
		encoder_button_longClick();
	}	

	gercon.tick();
	gerconCurrent = gercon.getState();
	if (gerconCurrent != gerconLast) {
		if (!gerconCurrent) {
			if (gerconActivated) {
				isOn = true;
			}
		} else {
			gerconActivated = true;
			offHeat();
		}
		gerconLast = gerconCurrent;
	}

	// Опрос термопары
	thermocoupleNewTime = millis() / thermocoupleTimeout;
	if (thermocoupleOldTime != thermocoupleNewTime)
	{
		thermocoupleTemperature = thermocouple.readCelsius();
		thermocoupleTemperature = correctionFormula;
		thermocoupleOldTime = thermocoupleNewTime;
	}

	// Аварийное выключение
	if(thermocoupleTemperature > temperatureEmergencyStop){
		offHeat();
	}

	if(isOn){
		pidNewTime = millis() / PID_Td;
		if (pidOldTime != pidNewTime)
		{
			PID.input = thermocoupleTemperature;
			PID.setpoint = currentTemperature;
			pwmSolder = PID.getResult();
			pidOldTime = pidNewTime;
		}
		pwmFan = map(fanSpeed, 0, 100, pwmFanMin, pwmFanMax);
	}else{
		pwmSolder = 0;
		pwmFan = 0;

		if(thermocoupleTemperature > temperatureWhenOffFanOff){
			if(thermocoupleTemperature > temperatureWhenOffFanOn){
				offFanComplete = false;
			}
			if(!offFanComplete){
				pwmFan = pwmFanMax;
			}
		}else{
			offFanComplete = true;
		}
	}

	digitalWrite(relayPin, isOn);

	// Запись значений в таймер (ШИМ)
	// analogWrite(...) имеет проблемы на 10bit-шим
	_SFR_BYTE(TCCR1A) |= _BV(COM1B1);
	OCR1B = pwmSolder;
	
	_SFR_BYTE(TCCR2A) |= _BV(COM2A1);
	OCR2A = pwmFan;

	refreshDisplay();

	frametimeOldTime = frametimeNewTime;
	frametimeNewTime = millis();
	frametime = frametimeNewTime - frametimeOldTime;

	#ifdef DEBUG
		debugPrintData();
	#endif
}
