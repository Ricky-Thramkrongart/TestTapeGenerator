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
    I2C_eeprom i2C_eeprom;
    static std::vector<float64_t> fit64RV45_l;
    static std::vector<float64_t> fit64RV45_r;

private:
    void WriteFit64ToI2C_eeprom(uint16_t& addr_, std::vector<float64_t>& fit64) {
        i2C_eeprom.writeByte(addr_, fit64.size());
        uint8_t arraysize = i2C_eeprom.readByte(addr_);
        if (arraysize != fit64.size()) {
            Serial.println("I2C_eeprom Error");
        }
        addr_ += 1;
        uint8_t datasize = fit64.size() * sizeof(float64_t);
        i2C_eeprom.writeBlock(addr_, (uint8_t*)fit64.begin(), datasize);
        addr_ += datasize;
        Serial.print(F("fit64 written. addr: "));
        Serial.println(addr_);
    }
    bool ReadFit64FromI2C_eeprom(uint16_t& addr_, std::vector<float64_t>& fit64) {
        uint8_t arraysize = i2C_eeprom.readByte(addr_);
        Serial.print(F("Read Arraysize: "));
        Serial.println(arraysize);
        if (arraysize == 0 || arraysize == 0xFF || arraysize > 20) {
            return false;
        }
        addr_ += 1;
        fit64 = std::vector<float64_t>(arraysize);
        uint8_t datasize = fit64.size() * sizeof(float64_t);
        i2C_eeprom.readBlock(addr_, (uint8_t*)fit64.begin(), datasize);
        addr_ += datasize;
        Serial.print(F("fit64 read. addr: "));
        Serial.println(addr_);
        return true;
    }

public:
    void WriteFit64ToI2C_eeprom()
    {
        i2C_eeprom.begin();
        if (i2C_eeprom.isConnected()) {
            uint16_t addr(0);
            std::vector<std::vector<float64_t>*> fit64s = { &fit64RV45_l, &fit64RV45_r };
            for (std::vector<std::vector<float64_t>*>::iterator i = fit64s.begin(); fit64s.end() != i; i++) {
                if (!(*i)->empty()) {
                    WriteFit64ToI2C_eeprom(addr, **i);
                }
            }

        }
    }
    bool ReadFit64FromI2C_eeprom(void)
    {
        i2C_eeprom.begin();
        if (i2C_eeprom.isConnected()) {
            uint16_t addr(0);
            std::vector<std::vector<float64_t>*> fit64s = { &fit64RV45_l, &fit64RV45_r };
            for (std::vector<std::vector<float64_t>*>::iterator i = fit64s.begin(); fit64s.end() != i; i++) {
                bool result = ReadFit64FromI2C_eeprom(addr, **i);
                if (!result) {
                    Serial.println("ReadFit64FromI2C_eeprom Error");
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    dBMeter() :potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND), i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512)
    {
        potentio.begin();
        inputpregainRelay.Disable();

        //Determine Device
        if (fit64RV45_l.empty() || fit64RV45_r.empty()) {
            Device1();
            //bool I2C_eepromHasFitArray = ReadFit64FromI2C_eeprom();
            //if (false && !I2C_eepromHasFitArray) {
            //    Device2();
            //    SignalGenerator signalGenerator;
            //    System::UnmutedCalibrationMode();
            //    double dB = 0.0;
            //    signalGenerator.setFreq(1000, dB);
            //    Measurement m(dB, 45);
            //    GetdB(m);
            //    if (fabs(m.dBLeft - dB) > 1 || fabs(m.dBRight - dB) > 1) {
            //        Device1();
            //    }
            //    WriteFit64ToI2C_eeprom();
            //}
        }
    }
    ~dBMeter()
    {
    }

    void Device1(void) {
        fit64RV45_l = std::vector <float64_t>(15);
        fit64RV45_l[14] = fp64_atof(String(F("-1.677923585110501e-36")).c_str());
        fit64RV45_l[13] = fp64_atof(String(F("1.2560653304131632e-32")).c_str());
        fit64RV45_l[12] = fp64_atof(String(F("-4.227894224574707e-29")).c_str());
        fit64RV45_l[11] = fp64_atof(String(F("8.45064378915242e-26")).c_str());
        fit64RV45_l[10] = fp64_atof(String(F("-1.115804991376365e-22")).c_str());
        fit64RV45_l[9] = fp64_atof(String(F("1.0244379036302585e-19")).c_str());
        fit64RV45_l[8] = fp64_atof(String(F("-6.705683940884093e-17")).c_str());
        fit64RV45_l[7] = fp64_atof(String(F("3.158402453382945e-14")).c_str());
        fit64RV45_l[6] = fp64_atof(String(F("-1.0682418823283996e-11")).c_str());
        fit64RV45_l[5] = fp64_atof(String(F("2.566411400535807e-09")).c_str());
        fit64RV45_l[4] = fp64_atof(String(F("-4.3066302928320864e-07")).c_str());
        fit64RV45_l[3] = fp64_atof(String(F("4.9748269797742075e-05")).c_str());
        fit64RV45_l[2] = fp64_atof(String(F("-0.004030972379686488")).c_str());
        fit64RV45_l[1] = fp64_atof(String(F("0.28074280452148")).c_str());
        fit64RV45_l[0] = fp64_atof(String(F("-29.355779672332574")).c_str());

        fit64RV45_r = std::vector <float64_t>(15);
        fit64RV45_r[14] = fp64_atof(String(F("-2.1287535929115348e-36")).c_str());
        fit64RV45_r[13] = fp64_atof(String(F("1.5748997961393054e-32")).c_str());
        fit64RV45_r[12] = fp64_atof(String(F("-5.235180407213225e-29")).c_str());
        fit64RV45_r[11] = fp64_atof(String(F("1.0326522313620901e-25")).c_str());
        fit64RV45_r[10] = fp64_atof(String(F("-1.3446707593535264e-22")).c_str());
        fit64RV45_r[9] = fp64_atof(String(F("1.2167590034249164e-19")).c_str());
        fit64RV45_r[8] = fp64_atof(String(F("-7.84515584960287e-17")).c_str());
        fit64RV45_r[7] = fp64_atof(String(F("3.6376381048261166e-14")).c_str());
        fit64RV45_r[6] = fp64_atof(String(F("-1.2103995770781873e-11")).c_str());
        fit64RV45_r[5] = fp64_atof(String(F("2.8579468570036992e-09")).c_str());
        fit64RV45_r[4] = fp64_atof(String(F("-4.704431961655748e-07")).c_str());
        fit64RV45_r[3] = fp64_atof(String(F("5.3107745275726005e-05")).c_str());
        fit64RV45_r[2] = fp64_atof(String(F("-0.004178304781667345")).c_str());
        fit64RV45_r[1] = fp64_atof(String(F("0.28128607980332293")).c_str());
        fit64RV45_r[0] = fp64_atof(String(F("-29.084007135068532")).c_str());
    }


    void Device2(void) {
        fit64RV45_l = std::vector <float64_t>(15);
        fit64RV45_l[14] = fp64_atof(String(F("-1.3012260559125937e-35")).c_str());
        fit64RV45_l[13] = fp64_atof(String(F("9.494262059115952e-32")).c_str());
        fit64RV45_l[12] = fp64_atof(String(F("-3.1268682744213197e-28")).c_str());
        fit64RV45_l[11] = fp64_atof(String(F("6.146799573438062e-25")).c_str());
        fit64RV45_l[10] = fp64_atof(String(F("-8.037299976392854e-22")).c_str());
        fit64RV45_l[9] = fp64_atof(String(F("7.374707899901929e-19")).c_str());
        fit64RV45_l[8] = fp64_atof(String(F("-4.88300808491803e-16")).c_str());
        fit64RV45_l[7] = fp64_atof(String(F("2.3633748281711946e-13")).c_str());
        fit64RV45_l[6] = fp64_atof(String(F("-8.379425770217733e-11")).c_str());
        fit64RV45_l[5] = fp64_atof(String(F("2.1609842284031454e-08")).c_str());
        fit64RV45_l[4] = fp64_atof(String(F("-3.986407205204476e-06")).c_str());
        fit64RV45_l[3] = fp64_atof(String(F("0.000511188364967593")).c_str());
        fit64RV45_l[2] = fp64_atof(String(F("-0.04357093118373209")).c_str());
        fit64RV45_l[1] = fp64_atof(String(F("2.329489728986675")).c_str());
        fit64RV45_l[0] = fp64_atof(String(F("-82.46352504265981")).c_str());

        fit64RV45_r = std::vector <float64_t>(15);
        fit64RV45_r[14] = fp64_atof(String(F("-1.4436231649486664e-35")).c_str());
        fit64RV45_r[13] = fp64_atof(String(F("1.0489683655067018e-31")).c_str());
        fit64RV45_r[12] = fp64_atof(String(F("-3.4395003093908697e-28")).c_str());
        fit64RV45_r[11] = fp64_atof(String(F("6.729451414975763e-25")).c_str());
        fit64RV45_r[10] = fp64_atof(String(F("-8.754188106097561e-22")).c_str());
        fit64RV45_r[9] = fp64_atof(String(F("7.987655610880187e-19")).c_str());
        fit64RV45_r[8] = fp64_atof(String(F("-5.256314114862736e-16")).c_str());
        fit64RV45_r[7] = fp64_atof(String(F("2.5266654527916786e-13")).c_str());
        fit64RV45_r[6] = fp64_atof(String(F("-8.89000755255239e-11")).c_str());
        fit64RV45_r[5] = fp64_atof(String(F("2.2731008508940537e-08")).c_str());
        fit64RV45_r[4] = fp64_atof(String(F("-4.15342530472971e-06")).c_str());
        fit64RV45_r[3] = fp64_atof(String(F("0.000527040998067761")).c_str());
        fit64RV45_r[2] = fp64_atof(String(F("-0.04441480169308953")).c_str());
        fit64RV45_r[1] = fp64_atof(String(F("2.3465342934877804")).c_str());
        fit64RV45_r[0] = fp64_atof(String(F("-82.24086926718981")).c_str());
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
        m.dBLeft = PolyVal(fit64RV45_l, m.RawLeft);
        m.dBRight = PolyVal(fit64RV45_r, m.RawRight);
        if (m.dBLeft < -28 || m.dBRight < -28) {
            inputpregainRelay.Enable();
            Measurement n(m);
            (this->*GetRawInput)(n);
            if (m.dBLeft < -28) {
                m.RawLeftGain = true;
                m.RawLeft = n.RawLeft;
                m.dBLeft = PolyVal(fit64RV45_l, m.RawLeft) - 12.0;
            }
            if (m.dBRight < -28) {
                m.RawRightGain = true;
                m.RawRight = n.RawRight;
                m.dBRight = PolyVal(fit64RV45_r, m.RawRight) - 12.0;
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

std::vector<float64_t> dBMeter::fit64RV45_l;
std::vector<float64_t> dBMeter::fit64RV45_r;
