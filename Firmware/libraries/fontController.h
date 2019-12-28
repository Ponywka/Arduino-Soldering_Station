#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*
    Формат шрифта:

    Байт | Описание
    -----+----------------------------------------------
      0  | Высота одного символа в пикселях
      1  | Ширина одного символа в пикселях
      2  | Вес одной строки в байтах
      3  | Количество символов
    4-.. | Массив символов (длина задана в 3-ем байте)
    ..-..| Массив несжатой картинки (длина до конца)
*/

class FontManager {
    public:
        static void drawText(Adafruit_SSD1306 display, int x, int y, const uint8_t *font, char *letters, int color = 1){
            // Вычисление размера изображения в символе [в байтах]
            int oneCharSize = pgm_read_byte(font + 1) * pgm_read_byte(font + 2);
            uint8_t character[oneCharSize]; // Массив с изображением символа

            // Поиск символов и вывод их на дисплей
            int charPos = 0;
            for(int letterID=0; letterID < strlen(letters); letterID++){
                // Новое положение пикселя на дисплее (нужно для корректного отображения)
                int newX = x + pgm_read_byte(font) * letterID;
                // Перебор всех возможных символов в шрифте
                for(int i = 4; i < (4 + pgm_read_byte(font + 3)); i++){
                    if((int)letters[letterID] == pgm_read_byte(font + i)){
                        charPos = i-4;
                        break;
                    } 
                }
                // Установка необходимого региона откуда нужно брать изображение символа
                int offset = 4 + pgm_read_byte(font + 3) + (oneCharSize*charPos);
                for(int i = offset; i < (offset + oneCharSize); i++){
                    character[i - offset] = pgm_read_byte(font + i);
                }

                // Вывод символа на экран
                display.drawBitmap(newX, y, character, pgm_read_byte(font + 0), pgm_read_byte(font + 1), color);
            }
        }

        static void drawTextFormated(Adafruit_SSD1306 display, int x1, int y1, int x2, int y2, const uint8_t *font, char *letters, int type = 0, int color = 1){
            int regionWidth = x2 - x1;
            int regionHeight = y2 - y1;

            int textWidth = pgm_read_byte(font + 0) * strlen(letters);
            int textHeight = pgm_read_byte(font + 1);

            double xPos = (regionWidth - textWidth) / 2 + x1;
            double yPos = (regionHeight - textHeight) / 2 + y1;
            
            FontManager::drawText(display, (int)xPos, (int)yPos, font, letters, color);
        }
};