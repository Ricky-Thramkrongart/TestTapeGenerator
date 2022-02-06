#ifndef SELFTEST_H
#define SELFTEST_H

#include <Wire.h>
#include "dBMeter.h"

void selftest()
{
    LCD_Helper lcdHelper;
    lcdHelper.Line(0, String(F("Free Memory: ")) + String(freeMemory()));
    lcdHelper.Show(Serial);
    lcdHelper.Show(1000);

    byte devices[] = { 0x25 /*Disp*/, 0x2C/*DATT*/, 0x50/*EEPROM*/, 0x68/*RTC*/ };
    for (int i = 0; i != sizeof(devices) / sizeof(byte); i++)
    {
        Wire.begin();
        Wire.beginTransmission(devices[i]);
        if (Wire.endTransmission() != 0) {
            char stringbuffer[255];
            sprintf(stringbuffer, "Hardware Not Found at %02X", devices[i]);
            lcdHelper.Line(0, stringbuffer);
            lcdHelper.Show();
            exit(EXIT_FAILURE);
        }
    }
    LCD_Helper().Test();

    lcdHelper.Line(0, String(F("Free Memory: ")) + String(freeMemory()));
    lcdHelper.Line(1, F("DISP:OK EEPROM:OK RTC:OK DATT:OK"));
    lcdHelper.Line(2, F("Frequncy response test"));
    lcdHelper.Show();
    lcdHelper.Show(Serial);

    SignalGenerator signalGenerator;
    System::UnmutedCalibrationMode();
    dBMeter dbMeter;

    //std::vector<double> freqTest{ 300, 1000, 10000, 20000.0, 25000.0 };
    std::vector<double> freqTest{ 25000.0 };
    //std::vector<double> dbTest{ 0.0, 5.0, 10.0, 25.0, 30.0 };
    std::vector<double> dbTest{ 30.0 };
    for (std::vector<double>::iterator f = freqTest.begin(); f != freqTest.end(); f++) {
        for (std::vector<double>::iterator d = dbTest.begin(); d != dbTest.end(); d++) {
            signalGenerator.setFreq(*f, *d);
            dBMeter::Measurement m;
            m.dB = *d;
            double dBLeft, dBRight;
            dbMeter.GetdB(m, dBLeft, dBRight);
            lcdHelper.Line(0, String(F("Free Memory: ")) + String(freeMemory()));
            lcdHelper.Line(2, SignalGenerator::String(*f, *d));
            lcdHelper.Line(3, m.String());
            lcdHelper.Show();
            lcdHelper.Show(Serial);

        }
    }
    delay(2000);
}

#endif // SELFTEST_H
