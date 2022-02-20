#ifndef LCDHELPER_H
#define LCDHELPER_H

#include <LiquidCrystal_I2C.h>
#include <SafeString.h>

class LCD_Helper
{
private:
    //Disables copy constructor
    LCD_Helper(const LCD_Helper&) = delete;
    LCD_Helper(LCD_Helper&) = delete;
    //Disables assignment operator
    LCD_Helper& operator=(const LCD_Helper&) = delete;
    LCD_Helper& operator=(LCD_Helper&) = delete;
    char line[4][41];

public:
    LiquidCrystal_I2C lcd;
    LCD_Helper(bool initilize = true) : lcd(0x25, 40, 2)
    {
        for (int i = 0; i != 4; ++i) {
            line[i][0] = 0;
        }

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

    void Line(uint8_t index, SafeString& str) {
        cSFA(sfLine, line[index]);
        sfLine = str;
    }

    void Line(uint8_t index, uint8_t offeset, const String& str) {
        cSFA(sfLine, line[index]);
        for (auto i = sfLine.length(); i < offeset; ++i) {
            sfLine.print(' ');
        }
        sfLine.print(str.c_str());
    }

    void Line(uint8_t index, const char* str) {
        cSFA(sfLine, line[index]);
        sfLine = str;
    }

    void Line(uint8_t index, const __FlashStringHelper * str) {
        cSFA(sfLine, line[index]);  
        sfLine = str;               
    }

    void Line(uint8_t index, const String& str) {
        cSFA(sfLine, line[index]);
        sfLine = str.c_str();
    }

    void Show(unsigned long _delay = 0)
    {
        lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF
        digitalWrite(8, HIGH);
        lcd.setCursor(0, 0);
        lcd.clear();
        lcd.print(line[0]);
        lcd.setCursor(0, 1);
        lcd.print(line[1]);

        digitalWrite(8, LOW);
        lcd.setCursor(0, 0);
        lcd.clear();
        lcd.print(line[2]);
        lcd.setCursor(0, 1);
        lcd.print(line[3]);

        if (_delay) {
            delay(_delay);
        }
    };
  
    void Show(Stream& out)
    {
        for (int i = 0; i != 4; ++i) {
            out.println(line[i]);
        }
    };

    void Test()
    {
        for (int k = 0; k != 2; ++k)
        {
            for (int j = 0; j != 4; ++j) {
                for (int i = 0; i != 40; ++i) {
                    line[j][i] = i + 32 + j * 40 + k * 40 * 4;
                }
                line[j][40] = 0;
            }

            lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF

            digitalWrite(8, HIGH);
            lcd.setCursor(0, 0);
            lcd.clear();
            lcd.print(line[0]);
            lcd.print(line[1]);

            digitalWrite(8, LOW);
            lcd.setCursor(0, 0);
            lcd.clear();
            lcd.print(line[2]);
            lcd.print(line[3]);
            delay(1000);
        }
    }
};

#endif // LCDHELPER_H
