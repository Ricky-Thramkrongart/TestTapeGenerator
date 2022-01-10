// *********************************************************************
//  Test software for Analyzer
// *********************************************************************

#include <ArduinoSTL.h>
#include <bitset>
#include "TapeInfo.h"
#include "LCDHelper.h"
using namespace std;

void splashscreen()
{
  LCD_Helper lcdhelper;
  lcdhelper.line[0] = "Test Tape Generator";
  lcdhelper.line[1] = "  SW rev: 2022/01/02";
  lcdhelper.line[2] = "TapeList: 2022/01/02";
  lcdhelper.Show();
  delay(2000);
}

void selftest()
{
  LCD_Helper lcdhelper;
  lcdhelper.line[0] = "I2C test: OK";
  lcdhelper.Show();
  delay(750);

  lcdhelper.line[1] = "RTC:OK EEPROM:OK ATT:OK DISP:OK";
  lcdhelper.Show();
  delay(750);

  lcdhelper.line[2] = "Frequncy response test:";
  lcdhelper.line[3] = "20Hz to 25Khz +/- 0.1 dB :OK";
  lcdhelper.Show();
  delay(1000);
}

void tapeSelection()
{
  LCD_Helper lcdhelper;
  DDRL = B00000000; // all inputs PORT-L D42 til D49

  auto ti = TapeInfo::Begin();
  auto current = TapeInfo::End();
  do {
    if (current != ti) {
      TapeInfo *tapeInfo = TapeInfo::Get(ti);
      std::vector<std::string> strs = tapeInfo->ToString();
      delete tapeInfo;
      char stringbuffer[40];
      sprintf(stringbuffer, "Tape #%i Input %i", ti, PINL);
      lcdhelper.line[0] = strs[0];
      lcdhelper.line[1] = strs[1];
      lcdhelper.line[1] = strs[1];
      lcdhelper.line[3] = stringbuffer;
      lcdhelper.Show();
      current = ti;
    }
    do {
    } while (PINL == 0);
    enum Buttons {BN_UP, BN_DOWN, BN_RIGHT, BN_LEFT, BN_PAGEUP, BN_PAGEDOWN, BN_ESCAPE, BN_OK};

    std::bitset<8> b{PINL};
    if (b.test(BN_UP) || b.test(BN_RIGHT)) {
      ti++;
      delay(50);
    } else if (b.test(BN_DOWN) || b.test(BN_LEFT)) {
      ti--;
      delay(50);
    } else if (b.test(BN_PAGEUP)) {
      for (int i = 0; i != 10; ++i) {
        ti++;
      }
      delay(50);
    } else if (b.test(BN_PAGEDOWN)) {
      for (int i = 0; i != 10; ++i) {
        ti--;
      }
      delay(50);
    } else if (b.test(BN_ESCAPE) || b.test(BN_OK)) {
      return;
    }

  } while (1);
}

void setup() {
  splashscreen();
  selftest();
  tapeSelection();

}

void loop() {
}
