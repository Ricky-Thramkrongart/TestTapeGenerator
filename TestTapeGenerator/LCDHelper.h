#ifndef LCDHELPER_H
#define LCDHELPER_H

#include <ArduinoSTL.h>
#include <LiquidCrystal_I2C.h>

class LCD_Helper
{
  public:
    std::string line[4];
    LiquidCrystal_I2C lcd;
    LCD_Helper(): lcd(0x25, 40, 2)
    {
      pinMode(8, OUTPUT);  // disp line select
      lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF
      digitalWrite(8, HIGH);   // disp line

      // Only run lcd.init() once per run.
      static bool initilized = false;

      if (!initilized) {
        lcd.init();              // initialize the LCD
      }
      lcd.clear();             // clear Display
      digitalWrite(8, LOW);   // disp line
      if (!initilized) {
        lcd.init();              // initialize the LCD
        // lcd.init() completed
        initilized = true;
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
};

#endif // LCDHELPER_H
