#include <Adafruit_SSD1306.h>

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
    Adafruit_GFX &display;
    uint8_t *fontAddress;

public:
    FontController(Adafruit_GFX &disp) : display(disp){};

    void setFont(const uint8_t *f)
    {
        fontAddress = (uint8_t *)f;
        charWidth = pgm_read_byte(fontAddress + 0);
        charHeight = pgm_read_byte(fontAddress + 1);
        lineWeight = pgm_read_byte(fontAddress + 2);
        countCharacters = pgm_read_byte(fontAddress + 3);
    }

    void drawChar(int16_t x, int16_t y, char character, uint8_t color = 1)
    {
        // Поиск символа
        for (uint8_t charID = 0; charID < countCharacters; charID++)
        {
            if ((uint8_t)character == pgm_read_byte(fontAddress + 4 + charID))
            {
                // Вывод символа на экран
                for (uint16_t byteOffset = 0; byteOffset < (lineWeight * charHeight); byteOffset++)
                {
                    uint8_t outputData[1] = {0};
                    outputData[0] = pgm_read_byte(fontAddress + 4 + countCharacters + (lineWeight * charHeight) * charID + byteOffset);
                    int16_t newX = x + (byteOffset % lineWeight) * 8;
                    int16_t newY = y + byteOffset / lineWeight;
                    display.drawBitmap(newX, newY, outputData, 8, 1, color);
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
        int16_t startX = x;
        for (int charNumber = 0; charNumber < maxStringLine; charNumber++)
        {
            if (text[charNumber] == '\n')
            {
                x = startX;
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