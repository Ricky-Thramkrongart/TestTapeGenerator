#pragma once

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
            char stringbuffer[255];
            char sz_dB[8];
            dtostrf(dB, 4, 2, sz_dB);

            int bitsLeft = ceil(log(RawLeft) / log(2));
            int bitsRight = ceil(log(RawRight) / log(2));
            sprintf(stringbuffer, "RV:%i Raw:%s Left:%i Right:%i Bits (L/R): %i/%i ", RV, sz_dB, RawLeft, RawRight, bitsLeft, bitsRight);
            return stringbuffer;
        }
        std::string ToStringData() {
            char stringbuffer[255];
            char sz_dB[8];
            dtostrf(dB, 4, 2, sz_dB);
            sprintf(stringbuffer, "%i,%s,%i,%i", RV, sz_dB, RawLeft, RawRight);
            return stringbuffer;
        }
        String String(void) {
            cSF(sf_line, 41);
            sf_line.print(F("dBMeter: "));
            sf_line.print(dBLeft, 2, 5);
            sf_line.print(F("dB "));
            sf_line.print(dBRight, 2, 5);
            sf_line.print(F("dB "));
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
    I2C_eeprom i2C_eeprom;
    static std::vector<float64_t> fit64RV45_l;
    static std::vector<float64_t> fit64RV45_r;

public:
    std::vector<float64_t> fit64;
    void WriteFit64ToEEPROM(void)
    {

        //Add output fit data size
        int addrOffset = sizeof(byte) + FIT64_SIZE * sizeof(float64_t);
        byte size_ = fit64.size();
        EEPROM.put(addrOffset, size_);
        addrOffset += sizeof(size_);
        for (int i = 0; i != size_; i++)
        {
            EEPROM.put(addrOffset, fit64[i]);
            addrOffset += sizeof(float64_t);
        }
    }
    void ReadFit64FromEEPROM(void)
    {
        int addrOffset = sizeof(byte) + FIT64_SIZE * sizeof(float64_t);
        byte size_;
        EEPROM.get(addrOffset, size_);
        addrOffset += sizeof(size_);
        fit64.resize(size_);
        for (int i = 0; i != size_; i++)
        {
            EEPROM.get(addrOffset, fit64[i]);
            addrOffset += sizeof(float64_t);
        }
    }
    uint16_t InPutFit64(double dB)
    {
        float64_t dB64 = fp64_sd(dB);
        float64_t rv = fp64_add(fit64[0], fp64_mul(fit64[1], dB64));
        for (int i = FIT_ORDER; i != 1; i--)
        {
            rv = fp64_add(rv, fp64_mul(fit64[i], fp64_pow(dB64, fp64_sd(i))));
        }

        //rv = fp64_round(rv);
        rv = fp64_fmin(rv, fp64_sd(255));
        rv = fp64_fmax(rv, fp64_sd(0));
        return atoi(fp64_to_string(rv, 15, 2));
    }

    dBMeter() :potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND), i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512)
    {
        potentio.begin();
        inputpregainRelay.Disable();

        //Determine Device
        if (fit64RV45_l.empty() || fit64RV45_r.empty()) {
            Device2();
            SignalGenerator signalGenerator;
            System::UnmutedCalibrationMode();
            double dB = 0.0;
            signalGenerator.setFreq(1000, dB);
            Measurement m(dB, 45);
            double dBLeft, dBRight;
            GetdB(m, dBLeft, dBRight);
            if (dBLeft - dB > 1 || dBRight - dB > 1) {
                Device1();
            }
        }
    }
    ~dBMeter()
    {
    }

    void Device1(void) {
        fit64RV45_l = std::vector <float64_t>(15);
        fit64RV45_l[14] = fp64_atof("1.677923585110501e-36");
        fit64RV45_l[13] = fp64_atof("-1.2560653304131632e-32");
        fit64RV45_l[12] = fp64_atof("4.227894224574707e-29");
        fit64RV45_l[11] = fp64_atof("-8.45064378915242e-26");
        fit64RV45_l[10] = fp64_atof("1.115804991376365e-22");
        fit64RV45_l[9] = fp64_atof("-1.0244379036302585e-19");
        fit64RV45_l[8] = fp64_atof("6.705683940884093e-17");
        fit64RV45_l[7] = fp64_atof("-3.158402453382945e-14");
        fit64RV45_l[6] = fp64_atof("1.0682418823283996e-11");
        fit64RV45_l[5] = fp64_atof("-2.566411400535807e-09");
        fit64RV45_l[4] = fp64_atof("4.3066302928320864e-07");
        fit64RV45_l[3] = fp64_atof("-4.9748269797742075e-05");
        fit64RV45_l[2] = fp64_atof("0.004030972379686488");
        fit64RV45_l[1] = fp64_atof("-0.28074280452148");
        fit64RV45_l[0] = fp64_atof("29.355779672332574");

        fit64RV45_r = std::vector <float64_t>(15);
        fit64RV45_r[14] = fp64_atof("2.1287535929115348e-36");
        fit64RV45_r[13] = fp64_atof("-1.5748997961393054e-32");
        fit64RV45_r[12] = fp64_atof("5.235180407213225e-29");
        fit64RV45_r[11] = fp64_atof("-1.0326522313620901e-25");
        fit64RV45_r[10] = fp64_atof("1.3446707593535264e-22");
        fit64RV45_r[9] = fp64_atof("-1.2167590034249164e-19");
        fit64RV45_r[8] = fp64_atof("7.84515584960287e-17");
        fit64RV45_r[7] = fp64_atof("-3.6376381048261166e-14");
        fit64RV45_r[6] = fp64_atof("1.2103995770781873e-11");
        fit64RV45_r[5] = fp64_atof("-2.8579468570036992e-09");
        fit64RV45_r[4] = fp64_atof("4.704431961655748e-07");
        fit64RV45_r[3] = fp64_atof("-5.3107745275726005e-05");
        fit64RV45_r[2] = fp64_atof("0.004178304781667345");
        fit64RV45_r[1] = fp64_atof("-0.28128607980332293");
        fit64RV45_r[0] = fp64_atof("29.084007135068532");
    }


    void Device2(void) {
        fit64RV45_l = std::vector <float64_t>(15);
        fit64RV45_l[14] = fp64_atof("1.3012260559125937e-35");
        fit64RV45_l[13] = fp64_atof("-9.494262059115952e-32");
        fit64RV45_l[12] = fp64_atof("3.1268682744213197e-28");
        fit64RV45_l[11] = fp64_atof("-6.146799573438062e-25");
        fit64RV45_l[10] = fp64_atof("8.037299976392854e-22");
        fit64RV45_l[9] = fp64_atof("-7.374707899901929e-19");
        fit64RV45_l[8] = fp64_atof("4.88300808491803e-16");
        fit64RV45_l[7] = fp64_atof("-2.3633748281711946e-13");
        fit64RV45_l[6] = fp64_atof("8.379425770217733e-11");
        fit64RV45_l[5] = fp64_atof("-2.1609842284031454e-08");
        fit64RV45_l[4] = fp64_atof("3.986407205204476e-06");
        fit64RV45_l[3] = fp64_atof("-0.000511188364967593");
        fit64RV45_l[2] = fp64_atof("0.04357093118373209");
        fit64RV45_l[1] = fp64_atof("-2.329489728986675");
        fit64RV45_l[0] = fp64_atof("82.46352504265981");

        fit64RV45_r = std::vector <float64_t>(15);
        fit64RV45_r[14] = fp64_atof("1.4436231649486664e-35");
        fit64RV45_r[13] = fp64_atof("-1.0489683655067018e-31");
        fit64RV45_r[12] = fp64_atof("3.4395003093908697e-28");
        fit64RV45_r[11] = fp64_atof("-6.729451414975763e-25");
        fit64RV45_r[10] = fp64_atof("8.754188106097561e-22");
        fit64RV45_r[9] = fp64_atof("-7.987655610880187e-19");
        fit64RV45_r[8] = fp64_atof("5.256314114862736e-16");
        fit64RV45_r[7] = fp64_atof("-2.5266654527916786e-13");
        fit64RV45_r[6] = fp64_atof("8.89000755255239e-11");
        fit64RV45_r[5] = fp64_atof("-2.2731008508940537e-08");
        fit64RV45_r[4] = fp64_atof("4.15342530472971e-06");
        fit64RV45_r[3] = fp64_atof("-0.000527040998067761");
        fit64RV45_r[2] = fp64_atof("0.04441480169308953");
        fit64RV45_r[1] = fp64_atof("-2.3465342934877804");
        fit64RV45_r[0] = fp64_atof("82.24086926718981");
    }

    typedef void (dBMeter::* GetRawInputType)(Measurement&);
    double GetdB(Measurement& m, double& dBLeft, double& dBRight)
    {
        m.RV = 45;
        inputpregainRelay.Disable();
        GetRawInputType GetRawInput = System::GetCalibration() ? &GetRawInputInternal : &GetRawInputExternal;
        m.RawLeftGain = false;
        m.RawRightGain = false;
        (this->*GetRawInput)(m);
        m.dBLeft = PolyVal(fit64RV45_l, m.RawLeft);
        m.dBRight = PolyVal(fit64RV45_r, m.RawRight);
        if (m.dBLeft > 28 || m.dBRight > 28) {
            inputpregainRelay.Enable();
            Measurement n(m);
            (this->*GetRawInput)(n);
            if (m.dBLeft > 28) {
                m.RawLeftGain = true;
                m.RawLeft = n.RawLeft;
                m.dBLeft = PolyVal(fit64RV45_l, m.RawLeft) + 12.0;
            }
            if (m.dBRight > 28) {
                m.RawRightGain = true;
                m.RawRight = n.RawLeft;
                m.dBRight = PolyVal(fit64RV45_r, m.RawRight) + 12.0;
            }
            inputpregainRelay.Disable();
        }
        dBLeft = m.dBLeft;
        dBRight = m.dBRight;
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
            for (float d = 0.0; d < 32.1; d += 0.05) {
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
        for (float d = 0.0; d < 32.1; d += .1) {
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

std::vector<float64_t> dBMeter::fit64RV45_l;
std::vector<float64_t> dBMeter::fit64RV45_r;