#pragma once

#include "dBMeter.h"
#include "Dialog.h"


class dBMeterOkDialog : public DialogOk
{
    SignalGenerator signalGenerator;
    dBMeter dbMeter;
    std::pair<double, double> dBOut;
public:
    dBMeterOkDialog():dBOut{ -2.0, -2.0 }
    {
        System::UnmutedCalibrationMode();
        signalGenerator.setFreq(1000, dBOut);
    }
    ~dBMeterOkDialog()
    {
        System::PopRelayStack();
    }

    void FullUpdate() {
        lcdhelper.Line(0, F("dBMeter"));
        dBMeter::Measurement m(dBOut);
        dbMeter.GetdB(m);
        Serial.println(m.ToString().c_str());
        Serial.println(m.String(4));
        lcdhelper.Line(2, m.ToString().c_str());
        lcdhelper.Line(3, m.String(4));
        lcdhelper.Show(1000);
    }

    void Update() {

        dBMeter::Measurement m(dBOut);
        dbMeter.GetdB(m);
        Serial.println(m.ToString().c_str());
        Serial.println(m.String(4));

        digitalWrite(8, LOW);
        lcdhelper.lcd.setCursor(0, 0);
        lcdhelper.lcd.print(m.ToString().c_str());
        lcdhelper.lcd.setCursor(0, 1);
        lcdhelper.lcd.print(m.String(4));
    }

};

