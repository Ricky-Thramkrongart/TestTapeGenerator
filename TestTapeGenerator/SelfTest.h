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

    //for (double d = -16.0; d < 0.1; d += 0.1) {
    //    cSF(sf_line, 41);
    //    sf_line.print(F("dBm: ")); sf_line.print(d); sf_line.print(F(" "));
    //    sf_line.print(F("OutPutFit64: ")); sf_line.print(signalGenerator.OutPutFit64(d));
    //    Serial.println(sf_line.c_str());
    //    delay(50);
    //}

    //std::vector<uint32_t> freqTest{ 300, 1000, 10000, 20000, 25000 };
    std::vector<uint32_t> freqTest{ 25000 };
    //std::vector<double> dbTest{ {0.0,0.0}, {-5.0,-5.3}, {-9.5, -10.0}, {-25.0, -25.0}, {-30.0,-30.0} };
    std::vector<std::pair<double, double>> dbTest{ { -30.0, -20.0}, { -20.0, -30.0} };

    for (std::vector<uint32_t>::iterator f = freqTest.begin(); f != freqTest.end(); f++) {
        for (std::vector<std::pair<double, double>>::iterator d = dbTest.begin(); d != dbTest.end(); d++) {
            signalGenerator.setFreq(*f, d->first, d->second);
            dBMeter::Measurement m;
            m.dB = d->first;
            dbMeter.GetdB(m);
            lcdHelper.Line(0, String(F("Free Memory: ")) + String(freeMemory()));
            lcdHelper.Line(2, SignalGenerator::String(*f, d->first));
            lcdHelper.Line(3, m.String());
            lcdHelper.Show();
            lcdHelper.Show(Serial);

        }
    }
    double dbOut = -9.5;
    double dbIn = FindDb(signalGenerator, dbMeter, dbOut);
    dBMeter::Measurement m;
    m.dB = dbOut;
    dbMeter.GetdB(m);
    lcdHelper.Line(2, SignalGenerator::String(1000, dbOut, 2));
    lcdHelper.Line(3, m.String(2));
    lcdHelper.Show();
    delay(2000);
}