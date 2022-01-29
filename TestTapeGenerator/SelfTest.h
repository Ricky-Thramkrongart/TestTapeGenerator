#ifndef SELFTEST_H
#define SELFTEST_H

#include <Wire.h>
void selftest()
{
    LCD_Helper lcdhelper;
    lcdhelper.line[0] = "Self Test";
    lcdhelper.Show();
    Serial.println(lcdhelper.line[0].c_str());
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
    lcdhelper.line[2] = "Frequncy response test";
    lcdhelper.Show();
    Serial.println(lcdhelper.line[1].c_str());
    Serial.println(lcdhelper.line[2].c_str());

    SignalGenerator signalGenerator;
    signalGenerator.UnmutedCalibrationMode();
    dBMeter dbMeter;
    std::vector<double> freqTest{300, 1000, 10000, 20000.0, 25000.0};
    std::vector<double> dbTest{0.0, 5.0, 10.0, 25.0, 30.0};
    for (std::vector<double>::iterator f = freqTest.begin(); f != freqTest.end(); f++) {
        for (std::vector<double>::iterator d = dbTest.begin(); d != dbTest.end(); d++) {
            signalGenerator.setFreq(*f, *d);
            dBMeter::Measurement m;
            m.dB = *d;
            double dBLeft, dBRight;
            dbMeter.GetdB(m, dBLeft, dBRight);
            char sz_dBLeft[255];
            char sz_dBRight[255];
            dtostrf(dBLeft, 4, 2, sz_dBLeft);
            dtostrf(dBRight, 4, 2, sz_dBRight);
            char stringbuffer[256];
            char sz_freq[8];
            dtostrf(*f, 4, 2, sz_freq);
            char sz_db[8];
            dtostrf(*d, 4, 2, sz_db);
            sprintf(stringbuffer, "Measured dB(L/R): %s/%s", sz_dBLeft, sz_dBRight);
            lcdhelper.line[3] = stringbuffer;
            sprintf(stringbuffer, "Frequency: %s dB: %s", sz_freq, sz_db);
            lcdhelper.line[2] = stringbuffer;
            lcdhelper.Show();
            Serial.println(lcdhelper.line[2].c_str());
            Serial.println(lcdhelper.line[3].c_str());

        }
    }
    delay(2000);
}
#endif // SELFTEST_H
