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

        std::pair<double, double> dBOut;
        std::pair<double, double> dBIn;
        std::pair<double, double> Raw;
        std::pair<double, double> Std;
        uint8_t RV;

        Measurement() {}
        Measurement(const std::pair<double, double>& dB_, const uint8_t RV_ = 45) : dBOut(dB_), RV(RV_) {
        }

        String ToString(void) {
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

        String ToStringData(const uint8_t decs = 1) {
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
            if (Std.first > 0.01) {
                sf_line.print(F("~"));
                sf_line.print(Std.first, decs, 2 + decs);
            }
            //if (Is_dBIn_OutOfRange(dBIn.second))
            //    sf_line.print(F("ovf."));
            //else
            sf_line.print(F(" "));
            sf_line.print(dBIn.second, decs, 4 + decs);
            if (Std.second > 0.01) {
                sf_line.print(F("~"));
                sf_line.print(Std.second, decs, 2 + decs);
            }
            return sf_line.c_str();
        }
    };

    // for chip info see https://www.analog.com/en/products/ad9833.html
    // SPI code taken from https://github.com/MajicDesigns/MD_AD9833/
    static Relay inputpregainRelay;
    AD5254_asukiaaa potentio;

private:
    typedef CircularBuffer<Measurement, CIRCULARBUFFERSIZE> CirBufType;
    static CirBufType buffer1;
    static CirBufType buffer2;

    bool SwapChannels;
    bool ChannelsVerified;
public:
    dBMeter() :potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND), SwapChannels(false), ChannelsVerified(false)
    {
        potentio.begin();
        inputpregainRelay.Disable();
    }
    ~dBMeter()
    {
    }

    void Calc(CirBufType& buffer, Measurement& m)
    {
        using index_t = CirBufType::index_t;

        float64_t dBLeftSum64 = fp64_sd(0.0);
        float64_t dBRightSum64 = fp64_sd(0.0);

        //Mean
        for (index_t i = 0; i < buffer.size(); i++) {
            dBLeftSum64 = fp64_add(dBLeftSum64, fp64_sd(buffer[i].Raw.first));
            dBRightSum64 = fp64_add(dBRightSum64, fp64_sd(buffer[i].Raw.second));
        }
        float64_t dBLeftMean64 = fp64_div(dBLeftSum64, fp64_sd(buffer.size()));
        float64_t dBRightMean64 = fp64_div(dBRightSum64, fp64_sd(buffer.size()));
        m.Raw.first = fp64_ds(dBLeftMean64);
        m.Raw.second = fp64_ds(dBRightMean64);

        //Standard Deviation
        double dBLeftMean = fp64_ds(dBLeftMean64);
        double dBRightMean = fp64_ds(dBRightMean64);
        double dBLeftSum = 0.0;
        double dBRightSum = 0.0;
        for (index_t i = 0; i < buffer.size(); i++) {
            dBLeftSum += buffer[i].Std.first;
            dBRightSum += buffer[i].Std.second;
        }
        m.Std.first = dBLeftSum / buffer.size();
        m.Std.second = dBRightSum / buffer.size();
    }

    typedef void (dBMeter::* GetRawInputType)(Measurement&);
    double GetdB(Measurement& m)
    {
        m.RV = 45;
        inputpregainRelay.Disable();
        GetRawInputType GetRawInput = System::GetCalibration() ? &GetRawInputInternal : &GetRawInputExternal;
        long ms = millis();
        bool retry;
        buffer1.clear();
        int counter = 0;
        do {
            (this->*GetRawInput)(m);
            m.dBIn.first = System::PolyVal(System::fit64RV45_l, System::fit64RV45_l_size, m.Raw.first, -System::_5dBInputAttenuator.first);
            m.dBIn.second = System::PolyVal(System::fit64RV45_r, System::fit64RV45_r_size, m.Raw.second, -System::_5dBInputAttenuator.second);

            std::pair<double, double> Std;
            Std.first = System::PolyVal(System::fit64RV45_l, System::fit64RV45_l_size, m.Raw.first + m.Std.first, -System::_5dBInputAttenuator.first) - m.dBIn.first;
            Std.second = System::PolyVal(System::fit64RV45_r, System::fit64RV45_r_size, m.Raw.second + m.Std.second, -System::_5dBInputAttenuator.second) - m.dBIn.second;

            if (m.dBIn.first < DBIN_MIN_NOPREGAIN || m.dBIn.second < DBIN_MIN_NOPREGAIN) {
                inputpregainRelay.Enable();
                Measurement n(m);
                (this->*GetRawInput)(n);
                if (m.dBIn.first < DBIN_MIN_NOPREGAIN) {
                    m.Raw.first = n.Raw.first;
                    m.dBIn.first = System::PolyVal(System::fit64RV45_l, System::fit64RV45_l_size, m.Raw.first, -System::_5dBInputAttenuator.first - 12.0);
                    Std.first = System::PolyVal(System::fit64RV45_l, System::fit64RV45_l_size, m.Raw.first + m.Std.first, -System::_5dBInputAttenuator.first - 12.0) - m.dBIn.first;
                }
                if (m.dBIn.second < DBIN_MIN_NOPREGAIN) {
                    m.Raw.second = n.Raw.second;
                    m.dBIn.second = System::PolyVal(System::fit64RV45_r, System::fit64RV45_r_size, m.Raw.second, -System::_5dBInputAttenuator.second - 12.0);
                    Std.second = System::PolyVal(System::fit64RV45_r, System::fit64RV45_r_size, m.Raw.second + m.Std.second, -System::_5dBInputAttenuator.second - 12.0) - m.dBIn.second;
                }
                inputpregainRelay.Disable();
            }
            m.Std.first = Std.first;
            m.Std.second = Std.second;
            //Serial.println(m.String(3));
            if (m.Std.first < .4 && m.Std.second < .4) {
                buffer1.push(m);
                Serial.println(m.String(2));

            }
            ++counter;
        } while ((counter < 10) && (m.Std.first >= .4 || m.Std.second >= .4));
        Calc(buffer2, m);

        if (ChannelsVerified && SwapChannels) {
            std::swap<double>(m.dBIn.first, m.dBIn.second);
            std::swap<double>(m.Raw.first, m.Raw.second);
            std::swap<double>(m.Std.first, m.Std.second);
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
        buffer2.clear();;
        m.Std.first = 0.0;
        m.Std.second = 0.0;
        do {
            const int CH1(A0);
            const int CH2(A1);
            m.Raw.first = analogRead(CH1);
            m.Raw.second = analogRead(CH2);
            buffer2.push(m);
            delay(50);

            if (buffer2.isFull()) {
                using index_t = decltype(buffer2)::index_t;
                for (index_t i = 0; i < buffer2.size(); i++) {
                    if (buffer2[0].Raw.first != buffer2[i].Raw.first || buffer2[0].Raw.second != buffer2[i].Raw.second) {
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
        buffer1.clear();
        do {
            const int CH1(A0);
            const int CH2(A1);
            m.Raw.first = analogRead(CH1);
            m.Raw.second = analogRead(CH2);
            buffer1.push(m);
            delay(50);
        } while (!buffer1.isFull());
        Calc(buffer1, m);
    }

    void RVSweep()
    {
        LCD_Helper lcdhelper;
        lcdhelper.Line(0, F("dBMeter RVSweep"));
        lcdhelper.Show();
        System::InternalMeasurementOn();
        //std::vector<int> rv{45, 146, 255};
        std::vector<int> rv{ 45 };
        for (std::vector<int>::iterator r = rv.begin(); r != rv.end(); r++) {
            for (float d = 0.0; d > -32.1; d -= 0.05) {
                SignalGenerator::Get().setFreq(1000, { d, d });
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
        System::InternalMeasurementOn();;
        int i = 44;
        for (float d = 0.0; d > -32.1; d -= .1) {
            SignalGenerator::Get().setFreq(1000, { d, d });
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

    void Cabling()
    {
        if (!System::GetCalibration() && !ChannelsVerified) {
            System::OutPutOn();
            SignalGenerator::Get().setFreq(1000, { -10, -15 });
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
Relay dBMeter::inputpregainRelay(Relay(30, false, 2000));

void System::SetupDevice() {

    System::Device2();
    Serial.print("System::Device2(): ");

    LCD_Helper lcdhelper;
    lcdhelper.Line(0, F("Setting System Paramaters"));
    lcdhelper.Show(100);

    System::InternalMeasurementOn();
    std::pair<double, double> dB({ -2, -2 });
    SignalGenerator::Get().setFreq(1000, dB);
    dBMeter::Measurement m;
    dBMeter::Get().GetdB(m);
    System::_5dBInputAttenuator.first += m.dBIn.first - dB.first;
    System::_5dBInputAttenuator.second += m.dBIn.second - dB.second;

    if (fabs(m.dBIn.first - dB.first) > MAX_DEVICE_STD_DEV || fabs(m.dBIn.second - dB.second) > MAX_DEVICE_STD_DEV) {
        System::Device1();
        Serial.print("System::Device1(): ");
        SignalGenerator::Get().setFreq(1000, dB);
        dBMeter::Get().GetdB(m);
        System::_5dBInputAttenuator.first += m.dBIn.first - dB.first;
        System::_5dBInputAttenuator.second += m.dBIn.second - dB.second;
        delay(10);

        if (fabs(m.dBIn.first - dB.first) > MAX_DEVICE_STD_DEV || fabs(m.dBIn.second - dB.second) > MAX_DEVICE_STD_DEV) {
            exit(EXIT_FAILURE);
        }
    }

    System::PopRelayStack();
}



dBMeter::CirBufType dBMeter::buffer1;
dBMeter::CirBufType dBMeter::buffer2;
