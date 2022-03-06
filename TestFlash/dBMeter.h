#pragma once

#include "SignalGenerator.h"
#include <fp64lib.h>

double PolyVal(const std::vector <float64_t>& fit64, uint16_t v, double offset)
{
    float64_t x = fp64_sd(v);
    float64_t y = fp64_add(fit64[0], fp64_mul(fit64[1], x));
    for (int i = fit64.size() - 1; i != 1; i--)
    {
        y = fp64_add(y, fp64_mul(fit64[i], fp64_pow(x, fp64_sd(i))));
    }

    y = fp64_fmax(y, fp64_sd(-29.2));
    y = fp64_fmin(y, fp64_sd(0));

    return fp64_ds(y) + offset;
}

class dBMeter
{
private:
    dBMeter(const dBMeter&) = delete;
    dBMeter& operator=(const dBMeter&) = delete;

public:
    static dBMeter& Get()
    {
        static dBMeter* dbMeter = new dBMeter();
        return *dbMeter;
    }
    struct Measurement {
        bool IsSaturated(void) {
            const uint16_t Saturation = 1023;
            if (Raw.first == Saturation || Raw.second >= Saturation)
                return true;
            return false;
        }
        bool HasNull(void) {
            if (Raw.first == 0 || Raw.second == 0)
                return true;
            return false;
        }
        Measurement() {}
        Measurement(const std::pair<double, double>& dB_, const uint8_t RV_ = 45) : dBOut(dB_), RV(RV_) {
        }
        std::pair<double, double> dBOut;
        std::pair<double, double> dBIn;
        std::pair<double, double> Raw;
        std::pair<double, double> Std;
        uint8_t RV;
    };

    // for chip info see https://www.analog.com/en/products/ad9833.html
    // SPI code taken from https://github.com/MajicDesigns/MD_AD9833/
    AD5254_asukiaaa potentio;

public:
    void begin(void) {
        potentio.begin();

        if (fit64RV45_l.empty() || fit64RV45_r.empty()) {
            Serial.print("fit64RV45_l.empty() || fit64RV45_r.empty()");
            delay(1000);
            exit(EXIT_FAILURE);
        }
    }
    dBMeter() :potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND)
    {
    }
    ~dBMeter()
    {
    }

    typedef void (dBMeter::* GetRawInputType)(Measurement&);
    double GetdB(Measurement& m)
    {
        m.RV = 45;
        GetRawInputType GetRawInput = &GetRawInputExternal;
        long ms = millis();
        bool retry;
        int counter = 0;
        do {
            (this->*GetRawInput)(m);
            m.dBIn.first = PolyVal(fit64RV45_l, m.Raw.first, -_5dBInputAttenuator.first);
            m.dBIn.second = PolyVal(fit64RV45_r, m.Raw.second, -_5dBInputAttenuator.second);

            std::pair<double, double> Std;
            Std.first = PolyVal(fit64RV45_l, m.Raw.first + m.Std.first, -_5dBInputAttenuator.first) - m.dBIn.first;
            Std.second = PolyVal(fit64RV45_r, m.Raw.second + m.Std.second, -_5dBInputAttenuator.second) - m.dBIn.second;
            m.Std.first = Std.first;
            m.Std.second = Std.second;
            ++counter;
        } while ((counter < 10) && (m.Std.first >= .4 || m.Std.second >= .4));
    }

    void GetRawInputExternal(Measurement& m)
    {
        //Serial.println(F("GetRawInputExternal"));
        static uint8_t rv = 0;
        if (rv != m.RV) {
            rv = m.RV;
            const uint8_t leftChannelIn(2);
            const uint8_t rightChannelIn(3);
            potentio.writeRDAC(leftChannelIn, rv);
            potentio.writeRDAC(rightChannelIn, rv);
            delay(600);
        }
            const int CH1(A0);
            const int CH2(A1);
            m.Raw.first = analogRead(CH1);
            m.Raw.second = analogRead(CH2);
    }

};

dBMeter dbMeter;

void SetupDevice() {
    Serial.print("Device1(): ");
    std::pair<double, double> dB({ -2, -2 });
    signalGenerator.setFreq(1000, dB);
    
    dBMeter::Measurement m;
    dbMeter.GetdB(m);

    //if (fabs(m.dBIn.first - dB.first) > MAX_DEVICE_STD_DEV || fabs(m.dBIn.second - dB.second) > MAX_DEVICE_STD_DEV) {
    //    exit(EXIT_FAILURE);
    //}
}
