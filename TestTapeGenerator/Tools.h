#pragma once

#include "dBMeter.h"
#include "Dialog.h"

class dBMeterOkDialog : public DialogOk
{
    SignalGenerator signalGenerator;
    dBMeter dbMeter;
    std::pair<double, double> dBOut;
    unsigned long counter;
public:
    dBMeterOkDialog() :dBOut{ -2.0, -2.0 }, counter(0)
    {
        System::UnmutedCalibrationMode();
        signalGenerator.setFreq(1000, dBOut);
    }
    ~dBMeterOkDialog()
    {
        System::PopRelayStack();
    }

    void FullUpdate() {
        cSF(sf_line, 41);
        sf_line.print(F("dBMeter #"));
        sf_line.print(++counter);

        lcdhelper.Line(0, sf_line);
        dBMeter::Measurement m(dBOut);
        dbMeter.GetdB(m);
        Serial.println(m.ToString().c_str());
        Serial.println(m.String(4));
        lcdhelper.Line(2, m.ToString().c_str());
        lcdhelper.Line(3, m.String(4));
        lcdhelper.Show(1000);
    }

    void Update() {
        cSF(sf_line, 41);
        sf_line.print(F("dBMeter #"));
        sf_line.print(++counter);

        dBMeter::Measurement m(dBOut);
        dbMeter.GetdB(m);

        Serial.println(m.ToString().c_str());
        Serial.println(m.String(4));

        digitalWrite(8, HIGH);
        lcdhelper.lcd.setCursor(0, 0);
        lcdhelper.lcd.print(sf_line);
        digitalWrite(8, LOW);
        lcdhelper.lcd.setCursor(0, 0);
        lcdhelper.lcd.print(m.ToString().c_str());
        lcdhelper.lcd.setCursor(0, 1);
        lcdhelper.lcd.print(m.String(4));
    }

};


class SignalGeneratorOkDialog : public DialogOk
{
    SignalGenerator signalGenerator;
    dBMeter dbMeter;
    std::pair<double, double> dBOut;
    uint32_t frequencyOut;
    unsigned long counter;

public:
    SignalGeneratorOkDialog() :dBOut{ -2.0, -2.0 }, frequencyOut(1000), counter(0)
    {
        System::UnmutedCalibrationMode();
    }
    ~SignalGeneratorOkDialog()
    {
        System::PopRelayStack();
    }

    void FullUpdate() {
        lcdhelper.Line(0, F("Signal Generator"));
        lcdhelper.Line(1, F("Format: [float float float] 115200 Baud"));
        lcdhelper.Show();

        Serial.setTimeout(500);
        while (Serial.available() > 0) Serial.read();
        Serial.flush();
        lcdhelper.Show(Serial);
    }

    void Update() {
        if (Serial.available()) {
            MatchState ms;
            String str(Serial.readString());
            ms.Target(const_cast<char*>(str.c_str()));
            char result = ms.Match("([-+]?[0-9]*\.?[0-9]+) ([-+]?[0-9]*\.?[0-9]+) ([-+]?[0-9]*\.?[0-9]+)");
            char cap[256];

            if (result == REGEXP_MATCHED)
            {
                int index = 0;
                ms.GetCapture(cap, index++);
                int freq = atoi(cap);
                ms.GetCapture(cap, index++);
                double dBLeft = atof(cap);
                ms.GetCapture(cap, index++);
                double dBRight = atof(cap);
                std::pair<double, double> dB{ dBLeft , dBRight };
                signalGenerator.setFreq(freq, dB);
                dBMeter::Measurement m(dB);
                dbMeter.GetdB(m);
                lcdhelper.Line(2, SignalGenerator::String(freq, dB));
                lcdhelper.Line(3, m.String());
                lcdhelper.Show();
                lcdhelper.Show(Serial);
            }
            while (Serial.available() > 0) Serial.read();
        }
        delay(1000);
    }
};

void SetDateTime()
{
    LCD_Helper lcdhelper;
    RTC_Helper rtchelper;
    lcdhelper.Line(0, F("Reading Date Time from Serial Port"));
    lcdhelper.Line(1, F("Format: [yyyy/mm/dd HH.MM.SS] 115200"));
    lcdhelper.Show(Serial);
    lcdhelper.Show(100);
    Serial.setTimeout(500);
    while (Serial.available() > 0) Serial.read();
    Serial.flush();
    lcdhelper.Show(Serial);
    do {
        if (Serial.available() >= 19) {
            MatchState ms;
            String str(Serial.readString());
            ms.Target(const_cast<char*>(str.c_str()));
            char result = ms.Match("(%d%d%d%d)/(%d%d)/(%d%d) (%d%d)\.(%d%d)\.(%d%d)");
            char cap[256];
            if (result == REGEXP_MATCHED)
            {
                time_t t;
                tmElements_t tm;
                int index = 0;
                ms.GetCapture(cap, index++);
                uint16_t y = atoi(cap);
                tm.Year = CalendarYrToTm(atoi(cap));
                ms.GetCapture(cap, index++);
                tm.Month = atoi(cap);
                ms.GetCapture(cap, index++);
                tm.Day = atoi(cap);
                ms.GetCapture(cap, index++);
                tm.Hour = atoi(cap);
                ms.GetCapture(cap, index++);
                tm.Minute = atoi(cap);
                ms.GetCapture(cap, index++);
                tm.Second = atoi(cap);
                t = makeTime(tm);
                RTC.set(t);        // use the time_t value to ensure correct weekday is set
                setSyncProvider(RTC.get);   // the function to get the time from the RTC
                lcdhelper.Line(2, F("Recieved Date Time."));
                lcdhelper.Show();
                lcdhelper.Show(Serial);
                delay(2000);
                return;
            }
            while (Serial.available() > 0) Serial.read();
        }
    } while (true);
}