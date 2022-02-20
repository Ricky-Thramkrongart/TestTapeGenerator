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
    dBMeterOkDialog():dBOut{ -2.0, -2.0 }, counter(0)
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

