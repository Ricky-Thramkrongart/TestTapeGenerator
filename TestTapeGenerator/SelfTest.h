#pragma once

#include <Wire.h>
#include "dBMeter.h"
#include "findDb.h"


void SelfTest()
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
    lcdHelper.Show();
    lcdHelper.Show(Serial);

    System::OutPutOff(); System::PopRelayStack();
    System::OutPutOn(); System::PopRelayStack();
    System::UnmutedCalibrationMode(); System::PopRelayStack();

    delay(2000);
}

void FrequencyResponseTest(const __FlashStringHelper* HeadLine, const std::vector<uint32_t>& freqTest, const std::vector<std::pair<double, double>>& dbTest, const unsigned long delay_ = 1000)
{
    LCD_Helper lcdHelper;
    lcdHelper.Line(0, HeadLine);
    lcdHelper.Show();

    SignalGenerator signalGenerator;
    dBMeter dbMeter;

    System::UnmutedCalibrationMode();

    for (std::vector<uint32_t>::const_iterator f = freqTest.begin(); f != freqTest.end(); f++) {
        for (std::vector<std::pair<double, double>>::const_iterator d = dbTest.begin(); d != dbTest.end(); d++) {
            lcdHelper.Line(2, SignalGenerator::String(*f, *d));
            lcdHelper.Line(3, F(""));
            lcdHelper.Show();
            signalGenerator.setFreq(*f, *d);
            dBMeter::Measurement m(*d);
            dbMeter.GetdB(m);
            lcdHelper.Line(3, m.String());
            lcdHelper.Show(Serial);
            lcdHelper.Show(delay_);
        }
    }
    System::PopRelayStack();
}

void FrequencyResponseTest_1()
{
    FrequencyResponseTest(F("Frequency Response Test #1"), std::vector<uint32_t>{ 20, 20000 }, std::vector<std::pair<double, double>> { { 0.0, 0.0 } });
}

void FrequencyResponseTest_2()
{
    FrequencyResponseTest(F("Frequency Response Test #2"), std::vector<uint32_t>{ 1000 }, std::vector<std::pair<double, double>> { { 0.0, 0.0}, { -4.9, -5.1},  { -5.1, -4.9}, { -5.5, -5.5}, {-30.0, -20.0}, {-20.0, -30.0} });
}


void FrequencyResponseTest_3()
{
    FrequencyResponseTest(F("Frequency Response Test #3"), std::vector<uint32_t>{ 20, 315, 500, 1000, 5000, 10000, 15000, 18000, 20000, 22000 }, std::vector<std::pair<double, double>> { { -12.0, -12.0} }, 20000);
}

void FinddBTest()
{
    LCD_Helper lcdHelper;
    lcdHelper.Line(0, F("Find dB Test"));
    lcdHelper.Show();

    SignalGenerator signalGenerator;
    dBMeter dbMeter;

    System::UnmutedCalibrationMode();

    std::vector<std::pair<double, double>> dbFinddBTest{ { DBOUT_MAX_SERVICE, DBOUT_MAX_SERVICE }, {-15,-15}, { DBOUT_MIN_SERVICE, DBOUT_MIN_SERVICE } };
    std::vector<uint32_t> freqTest = { 1000 };
    for (std::vector<uint32_t>::iterator f = freqTest.begin(); f != freqTest.end(); f++) {
        for (std::vector<std::pair<double, double>>::iterator d = dbFinddBTest.begin(); d != dbFinddBTest.end(); d++) {
            lcdHelper.Line(1, F(""));
            lcdHelper.Line(2, SignalGenerator::String(*f, *d));
            lcdHelper.Line(3, F(""));
            lcdHelper.Show();
            double e = 0.0;
            std::pair<double, double> dbIn = FindDb(signalGenerator, dbMeter, *f, *d, *d, e, lcdHelper);
            signalGenerator.setFreq(*f, dbIn);
            dBMeter::Measurement m(dbIn);
            dbMeter.GetdB(m);
            lcdHelper.Line(2, SignalGenerator::String(*f, dbIn, 2));
            lcdHelper.Line(3, m.String(2));
            Serial.println(SignalGenerator::String(*f, dbIn, 2));
            Serial.println(m.String(2));
            lcdHelper.Show(1000);
        }
    }
    System::PopRelayStack();
}

class TestMenu : public Menu
{
public:
    TestMenu() : Menu(5) {}
    void FullUpdate() {
        const __FlashStringHelper* str = 0;
        switch (Current) {
        case 0:
            str = F("SelfTest");
            break;
        case 1:
            str = F("Frequency Response Test #1");
            break;
        case 2:
            str = F("Frequency Response Test #2");
            break;
        case 3:
            str = F("Frequency Response Test #3");
            break;
        case 4:
            str = F("FinddB Test");
            break;
        }
        lcdhelper.Line(0, F("== Test Menu ====================="));
        lcdhelper.Line(1, str);
    }
};


void Tests()
{
    TestMenu testMenu;
    while (testMenu.Execute() == ButtonPanel<BasePanel>::IDOK) {
        switch (testMenu.Current)
        {
        case 0:
            SelfTest();
            break;
        case 1:
            FrequencyResponseTest_1();
            break;
        case 2:
            FrequencyResponseTest_2();
            break;
        case 3:
            FrequencyResponseTest_3();
            break;
        case 4:
            FinddBTest();
            break;
        };
    }
}
