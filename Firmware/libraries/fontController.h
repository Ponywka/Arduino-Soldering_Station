#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class FontManager {
    public:
        uint8_t fontG;

        void constructor(const uint8_t *font){
            fontG = font[0];
        };

        static void drawText(Adafruit_SSD1306 display, int x, int y, const uint8_t *font, char *letters, int color = 1){
            int oneCharSize = pgm_read_byte(font + 2) * pgm_read_byte(font + 1);
            uint8_t character[oneCharSize];
            int charPos = 0;
            for(int letterID=0; letterID < strlen(letters); letterID++){
                int newX = x + pgm_read_byte(font) * letterID;
                for(int i = 4; i < (4 + pgm_read_byte(font + 3)); i++){
                    if((int)letters[letterID] == pgm_read_byte(font + i)){
                        charPos = i-4;
                        break;
                    } 
                }

                // Нормальный оффсет
                int offset = 4 + pgm_read_byte(font + 3) + (oneCharSize*charPos);
                for(int i = offset; i < (offset + oneCharSize); i++){
                    character[i - offset] = pgm_read_byte(font + i);
                }

                display.drawBitmap(newX, y, character, pgm_read_byte(font + 0), pgm_read_byte(font + 1), color);
            }
        }
};