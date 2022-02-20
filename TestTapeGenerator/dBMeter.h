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

        std::string ToString(void) {
            int bitsLeft = ceil(log(Raw.first) / log(2));
            int bitsRight = ceil(log(Raw.second) / log(2));
            cSF(sf_line, 41);
            //sf_line.print(F("RV:")); sf_line.print(RV);
            //sf_line.print(F(" Out:")); sf_line.print(dBOut.first, 2, 4); sf_line.print(F(" ")); sf_line.print(dBOut.second, 2, 4);
            sf_line.print(F("Out:")); sf_line.print(dBOut.first, 2, 4); sf_line.print(F(" ")); sf_line.print(dBOut.second, 2, 4);
            sf_line.print(F(" Raw:")); sf_line.print(Raw.first, 0, 4); sf_line.print(F(" ")); sf_line.print(Raw.second, 0, 4);
            sf_line.print(F(" B:")); sf_line.print(bitsLeft); sf_line.print(F("/")); sf_line.print(bitsRight);
            return sf_line.c_str();
        }

        std::string ToStringData(const uint8_t decs = 1) {
            cSF(sf_line, 41);
            sf_line.print(RV);
            sf_line.print(F(","));
            if (Is_dBOut_OutOfRange(dBOut.first))
                sf_line.print(F("ovf"));
            else
                sf_line.print(dBOut.first, decs, 4 + decs);
            sf_line.print(F(","));
            if (Is_dBOut_OutOfRange(dBOut.second))
                sf_line.print(F("ovf"));
            else
                sf_line.print(dBOut.first, decs, 4 + decs);
            sf_line.print(F(","));
            sf_line.print(Raw.first);
            sf_line.print(F(","));
            sf_line.print(Raw.second);
            return sf_line.c_str();
        }

        String String(const uint8_t decs = 1) {
            cSF(sf_line, 41);
            sf_line.print(F("In: "));
            //if (Is_dBIn_OutOfRange(dBIn.first))
            //    sf_line.print(F("ovf."));
            //else
            sf_line.print(dBIn.first, decs, 4 + decs); 
            if (Std.first > 0.1) {
                sf_line.print(F("~"));
                sf_line.print(Std.first, decs, 2 + decs);
            }
            //if (Is_dBIn_OutOfRange(dBIn.second))
            //    sf_line.print(F("ovf."));
            //else
            sf_line.print(F(" "));
            sf_line.print(dBIn.second, decs, 4 + decs); 
            if (Std.second > 0.1) {
                sf_line.print(F("~"));
                sf_line.print(Std.second, decs, 2 + decs);
            }
            return sf_line.c_str();
        }
        std::pair<double, double> dBOut;
        std::pair<double, double> dBIn;
        std::pair<double, double> Raw;
        std::pair<double, double> Std;
        uint8_t RV;
    };

    // for chip info see https://www.analog.com/en/products/ad9833.html
    // SPI code taken from https://github.com/MajicDesigns/MD_AD9833/
    static Relay inputpregainRelay;
    AD5254_asukiaaa potentio;

private:
    bool SwapChannels;
    bool ChannelsVerified;
public:
    dBMeter() :potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND), SwapChannels(false), ChannelsVerified(false)
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
        long ms = millis();
        bool retry;
        do {
            (this->*GetRawInput)(m);
            m.dBIn.first = PolyVal(System::fit64RV45_l, m.Raw.first, -System::_5dBInputAttenuator.first);
            m.dBIn.second = PolyVal(System::fit64RV45_r, m.Raw.second, -System::_5dBInputAttenuator.second);
            if (m.dBIn.first < DBIN_MIN || m.dBIn.second < DBIN_MIN) {
                inputpregainRelay.Enable();
                Measurement n(m);
                (this->*GetRawInput)(n);
                if (m.dBIn.first < DBIN_MIN) {
                    m.Raw.first = n.Raw.first;
                    m.dBIn.first = PolyVal(System::fit64RV45_l, m.Raw.first, - System::_5dBInputAttenuator.first - 12.0);
                }
                if (m.dBIn.second < DBIN_MIN) {
                    m.Raw.second = n.Raw.second;
                    m.dBIn.second = PolyVal(System::fit64RV45_r, m.Raw.second, -System::_5dBInputAttenuator.second - 12.0);
                }
                inputpregainRelay.Disable();
            }
            if ((m.Std.first > SkinnersKonstant || m.Std.second > SkinnersKonstant) && millis() - ms < 5000) {
                Serial.println("m.Std.first > SkinnersKonstant || m.Std.second > SkinnersKonstant");
                delay(200); //settling time
                retry = true;
            }
            else
                retry = false;

        } while (retry);
        if (ChannelsVerified && SwapChannels) {
            std::swap<double>(m.dBIn.first, m.dBIn.second);
            std::swap<double>(m.Raw.first, m.Raw.second);
        }
    }

    void GetRawInputInternal(Measurement& m)
    {
        //Serial.println(F("GetRawInputInternal"));
        static uint8_t rv = 0;
        if (rv != m.RV) {
            rv = m.RV;
            const uint8_t leftChannelIn(2);
            const uint8_t rightChannelIn(3);
            potentio.writeRDAC(leftChannelIn, rv);
            potentio.writeRDAC(rightChannelIn, rv);
            delay(600);
        }

        bool measure_again;
        CircularBuffer<Measurement, CIRCULARBUFFERSIZE> buffer;
        m.Std.first = 0.0;
        m.Std.second = 0.0;
        do {
            const int CH1(A0);
            const int CH2(A1);
            m.Raw.first = analogRead(CH1);
            m.Raw.second = analogRead(CH2);
            buffer.push(m);
            delay(50);

            if (buffer.isFull()) {
                using index_t = decltype(buffer)::index_t;
                for (index_t i = 0; i < buffer.size(); i++) {
                    if (buffer[0].Raw.first != buffer[i].Raw.first || buffer[0].Raw.second != buffer[i].Raw.second) {
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

        CircularBuffer<Measurement, CIRCULARBUFFERSIZE> buffer;
        do {
            const int CH1(A0);
            const int CH2(A1);
            m.Raw.first = analogRead(CH1);
            m.Raw.second = analogRead(CH2);
            buffer.push(m);
            delay(50);
        } while (!buffer.isFull());

        using index_t = decltype(buffer)::index_t;
        float64_t dBLeftSum64 = fp64_sd(0.0);
        float64_t dBRightSum64 = fp64_sd(0.0);

        //Mean
        for (index_t i = 0; i < buffer.size(); i++) {
            dBLeftSum64 = fp64_add(dBLeftSum64, fp64_sd(buffer[i].Raw.first));
            dBRightSum64 = fp64_add(dBRightSum64, fp64_sd(buffer[i].Raw.second));
        }
        float64_t dBLeftMean64 = fp64_div(dBLeftSum64, fp64_sd(buffer.size()));
        float64_t dBRightMean64 = fp64_div(dBRightSum64, fp64_sd(buffer.size()));
        m.Raw.first = atoi(fp64_to_string(dBLeftMean64, 15, 2));
        m.Raw.second = atoi(fp64_to_string(dBRightMean64, 15, 2));

        //Standard Deviation
        double dBLeftMean = atof(fp64_to_string(dBLeftMean64, 15, 2));
        double dBRightMean = atof(fp64_to_string(dBRightMean64, 15, 2));
        double dBLeftSum = 0.0;
        double dBRightSum = 0.0;
        for (index_t i = 0; i < buffer.size(); i++) {
            dBLeftSum += square(buffer[i].Raw.first - dBLeftMean);
            dBRightSum += square(buffer[i].Raw.second - dBRightMean);
        }
        m.Std.first = sqrt(dBLeftSum / buffer.size());
        m.Std.second = sqrt(dBRightSum / buffer.size());
    }

    void RVSweep()
    {
        LCD_Helper lcdhelper;
        lcdhelper.Line(0, F("dBMeter RVSweep"));
        lcdhelper.Show();
        SignalGenerator signalGenerator;
        System::UnmutedCalibrationMode();
        //std::vector<int> rv{45, 146, 255};
        std::vector<int> rv{ 45 };
        for (std::vector<int>::iterator r = rv.begin(); r != rv.end(); r++) {
            for (float d = 0.0; d > -32.1; d -= 0.05) {
                signalGenerator.setFreq(1000, { d, d });
                Measurement m({ d,d }, *r);
                GetRawInputInternal(m);
                lcdhelper.lcd.setCursor(0, 0);
                lcdhelper.lcd.print(m.ToString().c_str());
                if (!m.HasNull() && !m.IsSaturated())
                    Serial.println(m.ToStringData().c_str());
            }
        }
        Serial.println("Finished");
        System::PopRelayStack();
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
            signalGenerator.setFreq(1000, { d, d });
            Measurement next({ d,d }, i);
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
        System::PopRelayStack();
    }

    void Cabling(SignalGenerator& signalGenerator)
    {
        if (!System::GetCalibration() && !ChannelsVerified) {
            System::OutPutOn();
            signalGenerator.setFreq(1000, { -10, -15 });
            Measurement m;
            GetdB(m);
            //Serial.println(SignalGenerator::String(1000, { -10, -15 }));
            //Serial.println(m.String(2).c_str());
            SwapChannels = (m.dBIn.first < m.dBIn.second);
            ChannelsVerified = true;
            //Serial.print(F("SwapChannels: ")); Serial.println(SwapChannels);
            System::PopRelayStack();
        }
    }
};
Relay dBMeter::inputpregainRelay(Relay(30));


void System::SetupDevice(void) {
    System::Device2();

    LCD_Helper lcdhelper;
    lcdhelper.Line(0, F("Setting System Paramaters"));
    lcdhelper.Show();

    SignalGenerator signalGenerator;
    dBMeter dbMeter;
    System::UnmutedCalibrationMode();
    std::pair<double, double> dB({ 0, 0 });
    signalGenerator.setFreq(1000, dB);
    dBMeter::Measurement m;
    dbMeter.GetdB(m);
    System::_5dBInputAttenuator.first += m.dBIn.first;
    System::_5dBInputAttenuator.second += m.dBIn.second;
    Serial.print(F("System::_5dBInputAttenuator: ")); Serial.println(m.String(6));
    Serial.println("System::Device2();");

    if (fabs(m.dBIn.first - dB.first) > 0.1 || fabs(m.dBIn.second - dB.second) > 0.1) {
        System::Device1();
        Serial.println("System::Device1();");
        signalGenerator.setFreq(1000, dB);
        dbMeter.GetdB(m);
        Serial.print(F("System::_5dBInputAttenuator: ")); Serial.println(m.String(6));
        System::_5dBInputAttenuator.first += m.dBIn.first;
        System::_5dBInputAttenuator.second += m.dBIn.second;
        if (fabs(m.dBIn.first - dB.first) > 0.1 || fabs(m.dBIn.second - dB.second) > 0.1) {
            exit(EXIT_FAILURE);
        }
    }
    System::PopRelayStack();
}
