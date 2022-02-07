#ifndef SPLASH_H
#define SPLASH_H

#include "LCDHelper.h"
#include "TestTapeGenerator.h"
#include "TapeInfo.h"

void splashscreen()
{
    LCD_Helper lcdhelper;
    cSF(sf_line, 41);
    sf_line.print(F("  "));
    sf_line.print(TESTTAPEGENERATOR_SW_VERSION);
    lcdhelper.Line(0, F("Test Tape Generator"));
    lcdhelper.Line(1, sf_line);
    sf_line.clear();
    sf_line.print(TAPELIST_VERSION);
    lcdhelper.Line(2, sf_line);
    lcdhelper.Show(Serial);
    lcdhelper.Show(2000);
}

#endif // SPLASH_H
