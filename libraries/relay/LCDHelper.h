#pragma once

#include <ArduinoSTL.h>
#include <LiquidCrystal_I2C.h>


class LCD_Helper
{
private:
    //Disables copy constructor
    LCD_Helper(const LCD_Helper&) = delete;
    LCD_Helper(LCD_Helper&) = delete;
    //Disables assignment operator
    LCD_Helper& operator=(const LCD_Helper&) = delete;
    LCD_Helper& operator=(LCD_Helper&) = delete;



public:
    std::string line[4];
    LiquidCrystal_I2C lcd;

    LCD_Helper() : lcd(0x25, 40, 2)
    {
        pinMode(8, OUTPUT);  // disp line select

        lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF

        digitalWrite(8, HIGH);   // disp line
        lcd.init();              // initialize the LCD
        lcd.clear();             // clear Display

        digitalWrite(8, LOW);   // disp line
        lcd.init();              // initialize the LCD
        lcd.clear();             // clear Display
    }

    void Init()
    {
        lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF

        digitalWrite(8, HIGH);   // disp line
        lcd.init();              // initialize the LCD
        lcd.clear();             // clear Display

        digitalWrite(8, LOW);   // disp line
        lcd.init();              // initialize the LCD
        lcd.clear();             // clear Display
    }

    static LCD_Helper& getInstance_()
    {
        static LCD_Helper    instance;  // Guaranteed to be destroyed.
        digitalWrite(11, HIGH); // set buzzer on
        delay(1000);
        digitalWrite(11, LOW); // set buzzer OFF
                                            // Instantiated on first use.
        instance.Init();
        return instance;
    }

    ~LCD_Helper() {
        digitalWrite(8, HIGH);   // disp line
        lcd.clear();             // clear Display
        digitalWrite(8, LOW);   // disp line
        lcd.clear();             // clear Display
    }

    void Show()
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
    }
    
    void Test()
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
            delay(1000);
        }
    }
};

extern LCD_Helper lcdHelper;
