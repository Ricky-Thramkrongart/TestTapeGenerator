#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H


#include <AD5254_asukiaaa.h>
#include <fp64lib.h>
#include <EEPROM.h>
#include <ArduinoSTL.h>
#include <algorithm>
#include <iterator>
#include <I2C_eeprom.h>


class Relay
{
protected:
    bool enabled;
    const uint8_t pin;
    bool reverse;

public:
    Relay(const uint8_t pin_, bool reverse_ = false) : pin(pin_), reverse(reverse_)
    {
        pinMode(pin, OUTPUT);
    }

	void Enable(void)
    {
        enabled = true;
		digitalWrite(pin, reverse ? LOW : HIGH);
    }

    void Disable(void)
    {
        enabled = false;
		digitalWrite(pin, reverse ? HIGH : LOW);
	}

    bool IsEnabled(void) 
    {
        return reverse ? !enabled : enabled;
    }
};


class System
{
protected:
    static Relay muteRelay;
    static Relay calibrationRelay;
public:
    static void UnMute()
    {
        muteRelay.Disable();
        calibrationRelay.Disable();
    }

    static void Mute()
    {
        muteRelay.Enable();
        calibrationRelay.Disable();
    }

    static void CalibrationMode()
    {
        muteRelay.Enable();
        calibrationRelay.Enable();
    }

    static void UnmutedCalibrationMode()
    {
        muteRelay.Disable();
        calibrationRelay.Enable();
    }

    static bool GetCalibration()
    {
        return  calibrationRelay.IsEnabled();
    }
    static bool GetMute()
    {
        return  muteRelay.IsEnabled();
    }
};
Relay System::muteRelay(Relay(28, true));
Relay System::calibrationRelay(Relay(26));

double PolyVal (const std::vector <float64_t>&fit64, uint16_t v)
{
    float64_t x = fp64_sd(v);
    float64_t y = fp64_add(fit64[0], fp64_mul(fit64[1], x));
    for (int i = fit64.size() - 1; i != 1 ; i--)
    {
        y = fp64_add(y, fp64_mul(fit64[i], fp64_pow(x, fp64_sd(i))));
    }

    y = fp64_fmin(y, fp64_sd(29.2));
    y = fp64_fmax(y, fp64_sd(0));

    return atof(fp64_to_string( y, 15, 2));
}

constexpr auto FIT_ORDER = 6;
#define FIT64_SIZE (FIT_ORDER+1)


class SignalGenerator
{
    protected:
        // for chip info see https://www.analog.com/en/products/ad9833.html
        // SPI code taken from https://github.com/MajicDesigns/MD_AD9833/
        static constexpr auto _clkPin = 13;
        static constexpr auto _fsyncPin = 2;
        static constexpr auto _dataPin = 9;
        AD5254_asukiaaa potentio;

public:
        std::vector<float64_t> fit64;
        void WriteFit64ToEEPROM (void)
        {
            int addrOffset = 0;
            byte size_ = fit64.size();
            EEPROM.put(addrOffset, size_);
            addrOffset += sizeof(size_);
            for (int i = 0; i != size_; i++)
            {
                EEPROM.put(addrOffset, fit64[i]);
                addrOffset += sizeof(float64_t);
            }
        }
        void ReadFit64FromEEPROM (void)
        {
            int addrOffset = 0;
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

        uint16_t OutPutFit64 (double dB)
        {
            float64_t dB64 = fp64_sd(dB);
            float64_t rv = fp64_add(fit64[0], fp64_mul(fit64[1], dB64));
            for (int i = FIT_ORDER; i != 1 ; i--)
            {
                rv = fp64_add(rv, fp64_mul(fit64[i], fp64_pow(dB64, fp64_sd(i))));
            }

            //rv = fp64_round(rv);
            rv = fp64_fmin(rv, fp64_sd(255));
            rv = fp64_fmax(rv, fp64_sd(0));
            return atoi(fp64_to_string( rv, 15, 2));
        }

        SignalGenerator(): potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND)
        {
            System::UnMute();

            ReadFit64FromEEPROM();

            potentio.begin();        // start Didital potmeter

            pinMode(_clkPin,   OUTPUT);
            digitalWrite(_clkPin,   LOW);

            pinMode(_fsyncPin, OUTPUT);
            digitalWrite(_fsyncPin, HIGH);

            pinMode(_dataPin,  OUTPUT);
        }
        ~SignalGenerator()
        {
            //Mute Output
        }

        void spiSend(const uint16_t data)
        {
            digitalWrite(_fsyncPin, LOW);

            uint16_t m = 1UL << 15;
            for (uint8_t i = 0; i < 16; i++)
            {
                digitalWrite(_dataPin, data & m ? HIGH : LOW);
                digitalWrite(_clkPin, LOW); //data is valid on falling edge
                digitalWrite(_clkPin, HIGH);
                m >>= 1;
            }
            digitalWrite(_dataPin, LOW); //idle low
            digitalWrite(_fsyncPin, HIGH);
        }

        void setdB(double dB)
        {
            double dBdiff;
            uint8_t output;
            if (dB < 5) {
                digitalWrite(7, LOW);  // -5 db att OFF
                digitalWrite(10, LOW);  // -10 db att OFF
                dBdiff = dB;
            } else if (dB >= 5 && dB < 10) {
                digitalWrite(7, HIGH);  // -5 db att ON
                digitalWrite(10, LOW);  // -10 db att OFF
                dBdiff = dB - 5;  // beregn rest att fra digi-pot
            } else if (dB >= 10 && dB < 15) {
                digitalWrite(7, LOW);   // -5 db att OFF
                digitalWrite(10, HIGH); // -10 db att ON
                dBdiff = dB - 10; // beregn rest att fra digi-pot
            }
            else if (dB >= 15) {
                digitalWrite(7, HIGH);  // -5 db att ON
                digitalWrite(10, HIGH); // -10 db att ON
                dBdiff = dB - 15 ; // beregn rest att fra digi-pot
            }
            output = OutPutFit64(dBdiff);
            const uint8_t leftChannelOut(0);
            const uint8_t rightChannelOut(1);
            potentio.writeRDAC(leftChannelOut, output);  //Right
            potentio.writeRDAC(rightChannelOut, output);  //Left
        }

        void ManualOutPut(uint8_t output)
        {
            setFreq(1000, 0); //ATTNUATOR OFF

            System::UnMute();

            const uint8_t leftChannelOut(0);
            const uint8_t rightChannelOut(1);
            potentio.writeRDAC(leftChannelOut, output);  //Right
            potentio.writeRDAC(rightChannelOut, output);  //Left
        }

        void setFreq(double f, double dB)
        {
            setdB(dB);

            const uint16_t b28  = (1UL << 13);
            const uint16_t freq = (1UL << 14);

            const double   f_clk = 25e6;
            const double   scale = 1UL << 28;
            const uint32_t n_reg = f * scale / f_clk;

            const uint16_t f_low  = n_reg         & 0x3fffUL;
            const uint16_t f_high = (n_reg >> 14) & 0x3fffUL;

            spiSend(b28);
            spiSend(f_low  | freq);
            spiSend(f_high | freq);
        }
};



#include <CircularBuffer.h>
#define CIRCULARBUFFERSIZE 30

class dBMeter
{
    public:
        struct Measurement {
            bool IsSaturated(void) {
                const uint16_t Saturation = 1023;
                if (dBLeft == Saturation || dBRight >= Saturation)
                    return true;
                return false;
            }
            bool HasNull(void) {
                if (dBLeft == 0 || dBRight == 0)
                    return true;
                return false;
            }
            Measurement() {}
            Measurement(const double dB_, const uint8_t RV_): dB(dB_), RV(RV_) {
            }
            std::string ToString(void) {
                char stringbuffer[255];
                char sz_dB[8];
                dtostrf(dB, 4, 2, sz_dB);

                int bitsLeft = ceil(log(dBLeft) / log(2));
                int bitsRight = ceil(log(dBRight) / log(2));
                sprintf(stringbuffer, "RV:%i dB:%s Left:%i Right:%i Bits (L/R): %i/%i " , RV, sz_dB, dBLeft, dBRight, bitsLeft, bitsRight);
                return stringbuffer;
            }
            std::string ToStringData() {
                char stringbuffer[255];
                char sz_dB[8];
                dtostrf(dB, 4, 2, sz_dB);
                sprintf(stringbuffer, "%i,%s,%i,%i" , RV, sz_dB, dBLeft, dBRight);
                return stringbuffer;
            }
            double dB;
            uint8_t RV;
            uint16_t dBLeft;
            uint16_t dBRight;
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
        void WriteFit64ToEEPROM (void)
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
        void ReadFit64FromEEPROM (void)
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
        uint16_t InPutFit64 (double dB)
        {
            float64_t dB64 = fp64_sd(dB);
            float64_t rv = fp64_add(fit64[0], fp64_mul(fit64[1], dB64));
            for (int i = FIT_ORDER; i != 1 ; i--)
            {
                rv = fp64_add(rv, fp64_mul(fit64[i], fp64_pow(dB64, fp64_sd(i))));
            }

            //rv = fp64_round(rv);
            rv = fp64_fmin(rv, fp64_sd(255));
            rv = fp64_fmax(rv, fp64_sd(0));
            return atoi(fp64_to_string( rv, 15, 2));
        }

        dBMeter():potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND), i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512)
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
            (this->*GetRawInput)(m);
            dBLeft = PolyVal(fit64RV45_l, m.dBLeft);
            dBRight = PolyVal(fit64RV45_r, m.dBRight);
            if (dBLeft > 28 || dBRight > 28) {
                inputpregainRelay.Enable();
                (this->*GetRawInput)(m);
                dBLeft = PolyVal(fit64RV45_l, m.dBLeft) + 12.0;
                dBRight = PolyVal(fit64RV45_r, m.dBRight) + 12.0;
				inputpregainRelay.Disable();
            }
        }

        void GetRawInputInternal(Measurement& m)
        {
            const uint8_t leftChannelIn(2);
            const uint8_t rightChannelIn(3);
            potentio.writeRDAC(leftChannelIn, m.RV);
            potentio.writeRDAC(rightChannelIn, m.RV);

            bool measure_again;
            CircularBuffer<Measurement, CIRCULARBUFFERSIZE> buffer;
            do {
                const int CH1(A0);
                const int CH2(A1);
                m.dBLeft = analogRead(CH1);
                m.dBRight = analogRead(CH2);
                buffer.push(m);
                delay(50);

                if (buffer.isFull()) {
                    using index_t = decltype(buffer)::index_t;
                    for (index_t i = 0; i < buffer.size(); i++) {
                        if (buffer[0].dBLeft != buffer[i].dBLeft || buffer[0].dBRight != buffer[i].dBRight) {
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
            const uint8_t leftChannelIn(2);
            const uint8_t rightChannelIn(3);
            potentio.writeRDAC(leftChannelIn, m.RV);
            potentio.writeRDAC(rightChannelIn, m.RV);
            delay(600);

            CircularBuffer<Measurement, CIRCULARBUFFERSIZE> buffer;
            do {
                const int CH1(A0);
                const int CH2(A1);
                m.dBLeft = analogRead(CH1);
                m.dBRight = analogRead(CH2);
                buffer.push(m);
                delay(50);
            } while (!buffer.isFull());

            using index_t = decltype(buffer)::index_t;
            float64_t dBLeftSum = fp64_sd(0.0);
            float64_t dBRightSum = fp64_sd(0.0);
            for (index_t i = 0; i < buffer.size(); i++) {
                dBLeftSum = fp64_add(dBLeftSum, fp64_sd(buffer[i].dBLeft));
                dBRightSum = fp64_add(dBRightSum, fp64_sd(buffer[i].dBRight));
            }
            float64_t dBLeftMean = fp64_div(dBLeftSum, fp64_sd(buffer.size()));
            float64_t dBRightMean = fp64_div(dBRightSum, fp64_sd(buffer.size()));
            m.dBLeft = atoi(fp64_to_string( dBLeftMean, 15, 2));
            m.dBRight = atoi(fp64_to_string( dBRightMean, 15, 2));
        }

        void RVSweep()
        {
            LCD_Helper lcdhelper;
            lcdhelper.line[0] = "dBMeter RVSweep";
            lcdhelper.Show();
            SignalGenerator signalGenerator;
            System::UnmutedCalibrationMode();;
            //std::vector<int> rv{45, 146, 255};
            std::vector<int> rv{45};
            for (std::vector<int>::iterator r = rv.begin(); r != rv.end(); r++) {
                for (float d = 0.0; d < 32.1; d += 0.05) {
                    signalGenerator.setFreq(1000.0, d);
                    Measurement m(d, *r);
                    GetRawInputInternal(m);
                    lcdhelper.lcd.setCursor(0, 0);
                    lcdhelper.lcd.print(m.ToString().c_str());
                    Beep();
                    if (!m.HasNull() && !m.IsSaturated())
                        Serial.println(m.ToStringData().c_str());
                }
            }
            Serial.println("Finished");
        }

        void Scan()
        {
            LCD_Helper lcdhelper;
            lcdhelper.line[0] = "dBMeter Scan";
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
#endif // SIGNALGENERATOR_H
