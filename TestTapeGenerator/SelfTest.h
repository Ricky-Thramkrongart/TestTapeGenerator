#pragma once

#include <Wire.h>
#include "dBMeter.h"
#include "findDb.h"

void selftest()
{
    LCD_Helper lcdHelper;
    byte devices[] = { 0x25 /*Disp*/, 0x2C/*DATT*/, 0x50/*EEPROM*/, 0x68/*RTC*/ };
    for (int i = 0; i != sizeof(devices) / sizeof(byte); i++)
    {
        Wire.begin();
        Wire.beginTransmission(devices[i]);
        if (Wire.endTransmission() != 0) {
            cSF(sf_line, 41);
            char stringbuffer[255];
            sf_line.print(F("Hardware Not Found at 0x"));
            sf_line.print(devices[i], HEX);
            lcdHelper.Line(0, sf_line);
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

    std::vector<uint32_t> freqTest{ 1000, 20000 };
    std::vector<std::pair<double, double>> dbTest{ { 0.0, 0.0}, { -4.9, -5.1},  { -5.1, -4.9}, { -5.5, -5.5}, { -30.0, -20.0}, { -20.0, -30.0} };
    for (std::vector<uint32_t>::iterator f = freqTest.begin(); f != freqTest.end(); f++) {
        for (std::vector<std::pair<double, double>>::iterator d = dbTest.begin(); d != dbTest.end(); d++) {
            signalGenerator.setFreq(*f, *d);
            dBMeter::Measurement m(*d);
            dbMeter.GetdB(m);
            lcdHelper.Line(0, String(F("Free Memory: ")) + String(freeMemory()));
            lcdHelper.Line(2, SignalGenerator::String(*f, *d));
            lcdHelper.Line(3, m.String());
            lcdHelper.Show();
            lcdHelper.Show(Serial);
        }
    }

    System::Mute(); System::PopRelayStack();
    System::UnMute(); System::PopRelayStack();
    System::UnmutedCalibrationMode(); System::PopRelayStack();
    System::CalibrationMode(); System::PopRelayStack();

    std::vector<std::pair<double, double>> dbFinddBTest{ { DBOUT_MAX_SERVICE, DBOUT_MAX_SERVICE }, { DBOUT_MIN_SERVICE, DBOUT_MIN_SERVICE },  { -9.4, -10.6 } };
    for (std::vector<uint32_t>::iterator f = freqTest.begin(); f != freqTest.end(); f++) {
        for (std::vector<std::pair<double, double>>::iterator d = dbFinddBTest.begin(); d != dbFinddBTest.end(); d++) {
            std::pair<double, double> dbIn = FindDb(signalGenerator, dbMeter, *f, *d);
            signalGenerator.setFreq(*f, dbIn);
            dBMeter::Measurement m(dbIn);
            dbMeter.GetdB(m);
            lcdHelper.Line(2, SignalGenerator::String(*f, dbIn, 2));
            lcdHelper.Line(3, m.String(2));
            lcdHelper.Show();
        }
    }
  
    delay(2000);
}