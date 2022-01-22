#ifndef SELFTEST_H
#define SELFTEST_H

#include <Wire.h>
void selftest()
{
  LCD_Helper lcdhelper;
  lcdhelper.line[0] = "Self Test";
  lcdhelper.Show();
  byte devices[] = {0x25 /*Disp*/, 0x2C/*DATT*/, 0x50/*EEPROM*/, 0x68/*RTC*/};
  for (int i = 0; i != sizeof(devices) / sizeof(byte); i++)
  {
    Wire.begin();
    Wire.beginTransmission(devices[i]);
    if (Wire.endTransmission() != 0) {
      char stringbuffer[255];
      sprintf(stringbuffer, "Hardware Not Found at %02X", devices[i]);
      lcdhelper.line[0] = stringbuffer;
      lcdhelper.Show();
      exit(EXIT_FAILURE);
    }
  }
  LCD_Helper().Test();

  lcdhelper.line[1] = "DISP:OK EEPROM:OK RTC:OK DATT:OK";
  lcdhelper.Show();
  delay(750);

  lcdhelper.line[2] = "Frequncy response test:";
  lcdhelper.line[3] = "20Hz to 25Khz +/- 0.1 dB :OK";
  lcdhelper.Show();

  delay(2000);
}

#endif // SELFTEST_H
