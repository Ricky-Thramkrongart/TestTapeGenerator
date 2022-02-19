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
    dBMeter dbMeter;

    System::UnmutedCalibrationMode();

    std::vector<uint32_t> freqTest{ 20,315,500, 1000,5000,10000,15000,18000, 20000,22000 };
    //  std::vector<std::pair<double, double>> dbTest{ { 0.0, 0.0}, { -4.9, -5.1},  { -5.1, -4.9}, { -5.5, -5.5}, { -30.0, -20.0}, { -20.0, -30.0} };
    std::vector<std::pair<double, double>> dbTest{ { -12.0, -12.0} };
    for (std::vector<uint32_t>::iterator f = freqTest.begin(); f != freqTest.end(); f++) {
        for (std::vector<std::pair<double, double>>::iterator d = dbTest.begin(); d != dbTest.end(); d++) {
            signalGenerator.setFreq(*f, *d);
            dBMeter::Measurement m(*d);
            dbMeter.GetdB(m);
            lcdHelper.Line(0, String(F("Free Memory: ")) + String(freeMemory()));
            lcdHelper.Line(2, SignalGenerator::String(*f, *d, 3));
            lcdHelper.Line(3, m.String(3));
            lcdHelper.Show(Serial);
            lcdHelper.Show(20000);
        }
    }
    /*
        System::Mute(); System::PopRelayStack();
        System::UnMute(); System::PopRelayStack();
        System::UnmutedCalibrationMode(); System::PopRelayStack();
        System::CalibrationMode(); System::PopRelayStack();

        std::vector<std::pair<double, double>> dbFinddBTest{ { DBOUT_MAX_SERVICE, DBOUT_MAX_SERVICE }, { DBOUT_MIN_SERVICE, DBOUT_MIN_SERVICE },  { -9.4, -10.6 } };
        for (std::vector<uint32_t>::iterator f = freqTest.begin(); f != freqTest.end(); f++) {
            for (std::vector<std::pair<double, double>>::iterator d = dbFinddBTest.begin(); d != dbFinddBTest.end(); d++) {
                double e = 0.0;
                std::pair<double, double> dbIn = FindDb(signalGenerator, dbMeter, *f, *d, *d, e);
                signalGenerator.setFreq(*f, dbIn);
                dBMeter::Measurement m(dbIn);
                dbMeter.GetdB(m);
                lcdHelper.Line(2, SignalGenerator::String(*f, dbIn, 2));
                lcdHelper.Line(3, m.String(2));
                lcdHelper.Show();
            }
        }

    */    delay(2000);
    System::PopRelayStack();
}


void FindDbTest()
{
    LCD_Helper lcdHelper;
    System::UnMute();
    SignalGenerator signalGenerator;
    dBMeter dbMeter;

    std::vector<uint32_t> freqTest{ 300, 1000, 3500, 10000, 15000, 20000, 25000 };
    std::vector<std::pair<double, double>> dbFinddBTest{ { DBOUT_MAX_SERVICE, DBOUT_MAX_SERVICE }, {-15,-15}, { DBOUT_MIN_SERVICE, DBOUT_MIN_SERVICE } };
    for (std::vector<uint32_t>::iterator f = freqTest.begin(); f != freqTest.end(); f++) {
        for (std::vector<std::pair<double, double>>::iterator d = dbFinddBTest.begin(); d != dbFinddBTest.end(); d++) {
            double e = 0.0;
            std::pair<double, double> dbIn = FindDb(signalGenerator, dbMeter, *f, *d, *d, e, lcdHelper);
            signalGenerator.setFreq(*f, dbIn);
            dBMeter::Measurement m(dbIn);
            dbMeter.GetdB(m);
            lcdHelper.Line(2, SignalGenerator::String(*f, dbIn, 2));
            lcdHelper.Line(3, m.String(2));
            Serial.println(SignalGenerator::String(*f, dbIn, 2));
            Serial.println(m.String(2));
            lcdHelper.Show();
        }
    }
}
