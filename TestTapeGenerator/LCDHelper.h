#ifndef LCDHELPER_H
#define LCDHELPER_H

#include <ArduinoSTL.h>
#include <LiquidCrystal_I2C.h>

class LCD_Helper
{
    public:
        std::string line[4];
        LiquidCrystal_I2C lcd;
        LCD_Helper(bool initilize = true): lcd(0x25, 40, 2)
        {
            initilize = true;
            pinMode(8, OUTPUT);  // disp line select
            lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF
            digitalWrite(8, HIGH);   // disp line

            if (initilize) {
                lcd.init();              // initialize the LCD
            }
            lcd.clear();             // clear Display
            digitalWrite(8, LOW);   // disp line
            if (initilize) {
                lcd.init();              // initialize the LCD
            }
            lcd.clear();             // clear Display
        };
        ~LCD_Helper() {
            digitalWrite(8, HIGH);   // disp line
            lcd.clear();             // clear Display
            digitalWrite(8, LOW);   // disp line
            lcd.clear();             // clear Display
        }
        Show()
        {
            for (int i = 0; i != 4; i++)
            {
                if (line[i].size() > 40) {
                    line[i].resize(40, ' ');
                }
            }

            lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF
            digitalWrite(8, HIGH);
            lcd.setCursor(0, 0);
            lcd.clear();
            lcd.print(line[0].c_str());
            lcd.setCursor(0, 1);
            lcd.print(line[1].c_str());

            digitalWrite(8, LOW);
            lcd.setCursor(0, 0);
            lcd.clear();
            lcd.print(line[2].c_str());
            lcd.setCursor(0, 1);
            lcd.print(line[3].c_str());
        };
        Test()
        {
            for (int k = 0; k != 2; ++k)
            {
                for (int j = 0; j != 4; ++j) {
                    line[j] = std::string(40, ' ');
                    for (int i = 0; i != 40; ++i) {
                        line[j][i] = i + 32 + j * 40 + k * 40 * 4;
                    }
                }

                lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF

                digitalWrite(8, HIGH);
                lcd.setCursor(0, 0);
                lcd.clear();
                lcd.print(line[0].c_str());
                lcd.print(line[1].c_str());

                digitalWrite(8, LOW);
                lcd.setCursor(0, 0);
                lcd.clear();
                lcd.print(line[2].c_str());
                lcd.print(line[3].c_str());
                delay(500);
            }
        }
};

#endif // LCDHELPER_H
