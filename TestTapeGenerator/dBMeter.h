#pragma once

#include "System.h"
#include "SignalGenerator.h"
#include <CircularBuffer.h>
constexpr auto CIRCULARBUFFERSIZE = 30;

class dBMeter
{
private:
    dBMeter(const dBMeter&) = delete;
    dBMeter& operator=(const dBMeter&) = delete;

public:
    struct Measurement {
        bool IsSaturated(void) {
            const uint16_t Saturation = 1023;
            if (RawLeft == Saturation || RawRight >= Saturation)
                return true;
            return false;
        }
        bool HasNull(void) {
            if (RawLeft == 0 || RawRight == 0)
                return true;
            return false;
        }
        Measurement() {}
        Measurement(const double dB_, const uint8_t RV_) : dB(dB_), RV(RV_) {
        }

        std::string ToString(void) {
            int bitsLeft = ceil(log(RawLeft) / log(2));
            int bitsRight = ceil(log(RawRight) / log(2));
            cSF(sf_line, 41);
            sf_line.print(F("RV:")); sf_line.print(RV);
            sf_line.print(F(" Raw:")); sf_line.print(dB, 2, 4);
            sf_line.print(F(" Left:")); sf_line.print(RawLeft);
            sf_line.print(F(" Right:")); sf_line.print(RawRight);
            sf_line.print(F(" Bits (L/R): ")); sf_line.print(bitsLeft);
            sf_line.print(F("/")); sf_line.print(bitsRight);
            return sf_line.c_str();
        }

        std::string ToStringData(uint8_t decs = 1) {
            cSF(sf_line, 41);
            sf_line.print(RV);
            sf_line.print(F(","));
            sf_line.print(dB, decs, 4);
            sf_line.print(F(","));
            sf_line.print(RawLeft);
            sf_line.print(F(","));
            sf_line.print(RawRight);
            return sf_line.c_str();
        }

        String String(uint8_t decs = 1) {


            cSF(sf_line, 41);
            sf_line.print(F("dBMeter: "));
            sf_line.print(dBLeft, decs, 5);
            sf_line.print(F("dBm "));
            sf_line.print(dBRight, decs, 5);
            sf_line.print(F("dBm "));
            return sf_line.c_str();
        }
        double dB;
        uint8_t RV;
        uint16_t RawLeft;
        uint16_t RawRight;
        bool RawLeftGain;
        bool RawRightGain;
        double dBLeft;
        double dBRight;
    };

    // for chip info see https://www.analog.com/en/products/ad9833.html
    // SPI code taken from https://github.com/MajicDesigns/MD_AD9833/
    static Relay inputpregainRelay;
    AD5254_asukiaaa potentio;

private:

public:
    dBMeter() :potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND)
    {
        potentio.begin();
        inputpregainRelay.Disable();

        if (System::fit64RV45_l.empty() || System::fit64RV45_r.empty()) {
            Serial.print("System::fit64RV45_l.empty() || System::fit64RV45_r.empty()");
            delay(1000);
            exit(EXIT_FAILURE);
        }
    }
    ~dBMeter()
    {
    }

    typedef void (dBMeter::* GetRawInputType)(Measurement&);
    double GetdB(Measurement& m)
    {
        m.RV = 45;
        inputpregainRelay.Disable();
        GetRawInputType GetRawInput = System::GetCalibration() ? &GetRawInputInternal : &GetRawInputExternal;
        m.RawLeftGain = false;
        m.RawRightGain = false;
        (this->*GetRawInput)(m);
        m.dBLeft = PolyVal(System::fit64RV45_l, m.RawLeft);
        m.dBRight = PolyVal(System::fit64RV45_r, m.RawRight);
        if (m.dBLeft < -28 || m.dBRight < -28) {
            inputpregainRelay.Enable();
            Measurement n(m);
            (this->*GetRawInput)(n);
            if (m.dBLeft < -28) {
                m.RawLeftGain = true;
                m.RawLeft = n.RawLeft;
                m.dBLeft = PolyVal(System::fit64RV45_l, m.RawLeft) - 12.0;
            }
            if (m.dBRight < -28) {
                m.RawRightGain = true;
                m.RawRight = n.RawRight;
                m.dBRight = PolyVal(System::fit64RV45_r, m.RawRight) - 12.0;
            }
            inputpregainRelay.Disable();
        }
    }

    void GetRawInputInternal(Measurement& m)
    {
        static uint8_t rv = 0;
        if (rv != m.RV) {
            rv = m.RV;
            const uint8_t leftChannelIn(2);
            const uint8_t rightChannelIn(3);
            potentio.writeRDAC(leftChannelIn, rv);
            potentio.writeRDAC(rightChannelIn, rv);
        }

        bool measure_again;
        CircularBuffer<Measurement, CIRCULARBUFFERSIZE> buffer;
        do {
            const int CH1(A0);
            const int CH2(A1);
            m.RawLeft = analogRead(CH1);
            m.RawRight = analogRead(CH2);
            buffer.push(m);
            delay(50);

            if (buffer.isFull()) {
                using index_t = decltype(buffer)::index_t;
                for (index_t i = 0; i < buffer.size(); i++) {
                    if (buffer[0].RawLeft != buffer[i].RawLeft || buffer[0].RawRight != buffer[i].RawRight) {
                        measure_again = true;
                        break;
                    }
                }
                measure_again = false;
            }
            else
                measure_again = true;
        } while (measure_again);
    }

    void GetRawInputExternal(Measurement& m)
    {
        static uint8_t rv = 0;
        if (rv != m.RV) {
            rv = m.RV;
            const uint8_t leftChannelIn(2);
            const uint8_t rightChannelIn(3);
            potentio.writeRDAC(leftChannelIn, rv);
            potentio.writeRDAC(rightChannelIn, rv);
            delay(600);
        }

        CircularBuffer<Measurement, CIRCULARBUFFERSIZE> buffer;
        do {
            const int CH1(A0);
            const int CH2(A1);
            m.RawLeft = analogRead(CH1);
            m.RawRight = analogRead(CH2);
            buffer.push(m);
            delay(50);
        } while (!buffer.isFull());

        using index_t = decltype(buffer)::index_t;
        float64_t dBLeftSum = fp64_sd(0.0);
        float64_t dBRightSum = fp64_sd(0.0);
        for (index_t i = 0; i < buffer.size(); i++) {
            dBLeftSum = fp64_add(dBLeftSum, fp64_sd(buffer[i].RawLeft));
            dBRightSum = fp64_add(dBRightSum, fp64_sd(buffer[i].RawRight));
        }
        float64_t dBLeftMean = fp64_div(dBLeftSum, fp64_sd(buffer.size()));
        float64_t dBRightMean = fp64_div(dBRightSum, fp64_sd(buffer.size()));
        m.RawLeft = atoi(fp64_to_string(dBLeftMean, 15, 2));
        m.RawRight = atoi(fp64_to_string(dBRightMean, 15, 2));
    }

    void RVSweep()
    {
        LCD_Helper lcdhelper;
        lcdhelper.Line(0, F("dBMeter RVSweep"));
        lcdhelper.Show();
        SignalGenerator signalGenerator;
        System::UnmutedCalibrationMode();;
        //std::vector<int> rv{45, 146, 255};
        std::vector<int> rv{ 45 };
        for (std::vector<int>::iterator r = rv.begin(); r != rv.end(); r++) {
            for (float d = 0.0; d > -32.1; d -= 0.05) {
                signalGenerator.setFreq(1000.0, d);
                Measurement m(d, *r);
                GetRawInputInternal(m);
                lcdhelper.lcd.setCursor(0, 0);
                lcdhelper.lcd.print(m.ToString().c_str());
                if (!m.HasNull() && !m.IsSaturated())
                    Serial.println(m.ToStringData().c_str());
            }
        }
        Serial.println("Finished");
    }

    void Scan()
    {
        LCD_Helper lcdhelper;
        lcdhelper.Line(0, F("dBMeter Scan"));
        lcdhelper.Show();
        SignalGenerator signalGenerator;
        System::UnmutedCalibrationMode();;
        int i = 44;
        for (float d = 0.0; d > -32.1; d -= .1) {
            signalGenerator.setFreq(1000.0, d);
            Measurement next(d, i);
            GetRawInputInternal(next);
            Measurement prev(next);
            while (next.RV != 255 && !next.IsSaturated()) {
                prev = next;
                next.RV += 1;
                GetRawInputInternal(next);
            };
            Measurement unSaturated(prev);
            if (!next.IsSaturated()) {
                unSaturated = next;
            }
            i = unSaturated.RV;
            lcdhelper.lcd.setCursor(0, 0);
            lcdhelper.lcd.print(unSaturated.ToString().c_str());
            Serial.println(unSaturated.ToString().c_str());
        }
        Serial.println("Finished");
    }
};
Relay dBMeter::inputpregainRelay(Relay(30));

