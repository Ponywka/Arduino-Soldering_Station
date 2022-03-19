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

enum AnchorPoint
{
    LeftTop,
    CenterTop,
    RightTop,
    LeftCenter,
    CenterCenter,
    RightCenter,
    LeftBottom,
    CenterBottom,
    RightBottom
};

enum TextPosition
{
    Left,
    Center,
    Right
};

class FontController
{
protected:
    int charWidth;            // Ширина одного символа [пиксели]
    int charHeight;           // Высота одного символа [пиксели]
    int lineWeight;           // Размер одной строки [байты]
    int countCharacters;      // Количество символов
    uint8_t tmpdata[1] = {0}; // Костыль для вывода одного байта за который надо отрезать руки автору
    Adafruit_SSD1306 &display;
    uint8_t *font;

public:
    FontController(Adafruit_SSD1306 &disp) : display(disp){};

    void setFont(const uint8_t *f)
    {
        font = (uint8_t *)f;
        charWidth = pgm_read_byte(font + 0);
        charHeight = pgm_read_byte(font + 1);
        lineWeight = pgm_read_byte(font + 2);
        countCharacters = pgm_read_byte(font + 3);
    }

    void drawChar(int16_t x, int16_t y, char character, uint8_t color = 1)
    {
        // Перебор всех символов
        // В случае нахождения нужного символа - вывод на экран
        for (uint8_t characterInArray = 0; characterInArray < countCharacters; characterInArray++)
        {
            if ((uint8_t)character == pgm_read_byte(font + 4 + characterInArray))
            {
                // Перебор всех байтов символа и вывод их на экран
                for (uint8_t byteBitmap = 0; byteBitmap < (lineWeight * charHeight); byteBitmap++)
                {
                    tmpdata[0] = pgm_read_byte(font + 4 + countCharacters + (lineWeight * charHeight) * characterInArray + byteBitmap);
                    display.drawBitmap(x + (byteBitmap % lineWeight) * 8, y + byteBitmap / lineWeight, tmpdata, 8, 1, color);
                }
                break;
            }
        }
    }

    void drawText(int16_t x, int16_t y, const char *text, uint8_t color = 1, int16_t maxStringLine = -1)
    {
        if (maxStringLine == -1)
        {
            maxStringLine = strlen(text);
        }
        int16_t oldX = x;
        for (int charNumber = 0; charNumber < maxStringLine; charNumber++)
        {
            if (text[charNumber] == '\n')
            {
                x = oldX;
                y += charHeight;
                continue;
            }
            drawChar(x, y, text[charNumber], color);
            x += charWidth;
        }
    }

    void drawTextFormated(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t anchorPoint, uint8_t textPosition, const char *text, uint8_t color = 1)
    {
        int16_t width = x2 - x1;
        int16_t height = y2 - y1;

        uint8_t lineCount = 1;
        uint8_t maxLineSize = 0;

        uint8_t thisLineSize = 0;
        for (uint8_t characterInText = 0; characterInText < strlen(text); characterInText++)
        {
            if (text[characterInText] == '\n')
            {
                lineCount++;
                thisLineSize = 0;
                continue;
            }
            thisLineSize++;
            if (thisLineSize > maxLineSize)
            {
                maxLineSize = thisLineSize;
            }
        }

        int16_t widthText = maxLineSize * charWidth;
        int16_t heightText = lineCount * charHeight;
        int16_t startX, startY;

        switch (anchorPoint)
        {
        case CenterTop:
            startX = (width - widthText) / 2;
            startY = 0;
            break;
        case RightTop:
            startX = width - widthText;
            startY = 0;
            break;
        case LeftCenter:
            startX = 0;
            startY = (height - heightText) / 2;
            break;
        case CenterCenter:
            startX = (width - widthText) / 2;
            startY = (height - heightText) / 2;
            break;
        case RightCenter:
            startX = width - widthText;
            startY = (height - heightText) / 2;
            break;
        case LeftBottom:
            startX = 0;
            startY = height - heightText;
            break;
        case CenterBottom:
            startX = (width - widthText) / 2;
            startY = height - heightText;
            break;
        case RightBottom:
            startX = width - widthText;
            startY = height - heightText;
            break;
        default:
            startX = 0;
            startY = 0;
            break;
        }
        startX += x1;
        startY += y1;
        
        // Для выравнивания текста по центру / правому краю
        int16_t newX, newY;
        int16_t thisLine = 0;
        char strings[lineCount][maxLineSize];
        uint16_t charsInLine[lineCount];
        if (textPosition)
        {
            thisLineSize = 0;
            for (uint8_t characterInText = 0; characterInText < strlen(text); characterInText++)
            {
                if (text[characterInText] == '\n')
                {
                    thisLine++;
                    thisLineSize = 0;
                    continue;
                }
                strings[thisLine][thisLineSize] = text[characterInText];
                thisLineSize++;
                charsInLine[thisLine] = thisLineSize;
            }
        }

        switch (textPosition)
        {
        case Center:
            for (uint8_t line; line < lineCount; line++)
            {
                newX = startX + (maxLineSize - charsInLine[line]) * charWidth / 2;
                newY = startY + line * charHeight;

                drawText(newX, newY, strings[line], color, charsInLine[line]);
            }
            break;
        case Right:
            for (uint8_t line; line < lineCount; line++)
            {
                newX = startX + (maxLineSize - charsInLine[line]) * charWidth;
                newY = startY + line * charHeight;
                drawText(newX, newY, strings[line], color, charsInLine[line]);
            }
            break;
        default:
            drawText(startX, startY, text, color);
            break;
        }
    }
};