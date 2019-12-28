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

class FontManager
{
public:
    static void drawText(Adafruit_SSD1306 display, int x, int y, const uint8_t *font, char *letters, int color = 1)
    {
        // Вычисление размера изображения в символе [в байтах]
        int oneCharSize = pgm_read_byte(font + 1) * pgm_read_byte(font + 2);
        uint8_t character[oneCharSize]; // Массив с изображением символа

        // Поиск символов и вывод их на дисплей
        int charPos = 0;

        int letterInLine = 0;
        int line = 0;

        for (int letterID = 0; letterID < strlen(letters); letterID++)
        {
            if ((int)letters[letterID] == '\n')
            {
                line++;
                letterInLine = 0;
                continue;
            }
            // Новое положение пикселя на дисплее (нужно для корректного отображения)
            int newX = x + pgm_read_byte(font) * letterInLine;
            int newY = y + pgm_read_byte(font + 1) * line;
            // Перебор всех возможных символов в шрифте
            for (int i = 4; i < (4 + pgm_read_byte(font + 3)); i++)
            {
                if ((int)letters[letterID] == pgm_read_byte(font + i))
                {
                    charPos = i - 4;
                    break;
                }
            }
            // Установка необходимого региона откуда нужно брать изображение символа
            int offset = 4 + pgm_read_byte(font + 3) + (oneCharSize * charPos);
            for (int i = offset; i < (offset + oneCharSize); i++)
            {
                character[i - offset] = pgm_read_byte(font + i);
            }

            // Вывод символа на экран
            display.drawBitmap(newX, newY, character, pgm_read_byte(font + 0), pgm_read_byte(font + 1), color);

            letterInLine++;
        }
    }

    static void drawTextFormated(Adafruit_SSD1306 display, int x1, int y1, int x2, int y2, const uint8_t *font, char *letters, int type = 0, int color = 1)
    {
        int regionWidth = x2 - x1;
        int regionHeight = y2 - y1;

        int maxSymbolsPerLine = 0;
        int symbolsInLine = 0;
        int lines = 1;
        for (int letterID = 0; letterID < strlen(letters); letterID++)
        {
            if ((int)letters[letterID] == '\n')
            {
                symbolsInLine=0;
                lines++;
            }
            symbolsInLine++;
            if(symbolsInLine>maxSymbolsPerLine){
                maxSymbolsPerLine = symbolsInLine;
            }
        }

        int textWidth = pgm_read_byte(font + 0) * maxSymbolsPerLine;
        int textHeight = pgm_read_byte(font + 1) * lines;

        double xPos = x1;
        double yPos = y1;
        

        switch (type)
        {
        case 1:
            xPos = (regionWidth - textWidth) / 2 + x1;
            yPos = (regionHeight - textHeight) / 2 + y1;
            break;
        
        case 2:
            xPos = (regionWidth - textWidth) + x1;
            yPos = (regionHeight - textHeight) / 2 + y1;

        default:
            yPos = (regionHeight - textHeight) / 2 + y1;
            break;
        }

        FontManager::drawText(display, (int)xPos, (int)yPos, font, letters, color);
    }
};