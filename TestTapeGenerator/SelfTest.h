#ifndef SELFTEST_H
#define SELFTEST_H

#include <Wire.h>
void selftest()
{
  LCD_Helper lcdhelper;
  lcdhelper.line[0] = "Self Test";
  lcdhelper.Show();
  byte devices[] = {0x25, 0x2C, 0x50, 0x68};
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

/*
  for (double d = 32.0; d >= 0.0; d -= 0.1) {
    char stringbuffer[255];
    char sz_d[8];
    dtostrf(d, 4, 1, sz_d);

    sprintf(stringbuffer, "%s %i" , sz_d, OutPutTableFit(d));
    Serial.println(stringbuffer);
    delay(500);
  }
*/
  delay(2000);
}

#endif // SELFTEST_H
