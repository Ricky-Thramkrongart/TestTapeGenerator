#ifndef SPLASH_H
#define SPLASH_H

#include "LCDHelper.h"
#include "TestTapeGenerator.h"
#include "TapeInfo.h"

void splashscreen()
{
    LCD_Helper lcdhelper;
    char stringbuffer[255];
    sprintf(stringbuffer, "  %s", TESTTAPEGENERATOR_SW_VERSION);
    lcdhelper.Line(0, F("Test Tape Generator"));
    lcdhelper.Line(1, stringbuffer);
    sprintf(stringbuffer, "%s", TAPELIST_VERSION);
    lcdhelper.Line(2, stringbuffer);
    lcdhelper.Show(Serial);
    lcdhelper.Show(2000);
}

#endif // SPLASH_H
