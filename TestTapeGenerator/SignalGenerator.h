#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include <AD5254_asukiaaa.h>
#include <fp64lib.h>
#include <EEPROM.h>
#include <ArduinoSTL.h>
#include <algorithm>
#include <iterator>
#include <I2C_eeprom.h>

#define FIT_ORDER 6
#define FIT64_SIZE (FIT_ORDER+1)
class SignalGenerator
{
    protected:
        // for chip info see https://www.analog.com/en/products/ad9833.html
        // SPI code taken from https://github.com/MajicDesigns/MD_AD9833/
        const uint8_t _clkPin;
        const uint8_t _fsyncPin;
        const uint8_t _dataPin;
        const uint8_t _outputonoffPin;
        const uint8_t _calibrationtonoffPin;
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

        SignalGenerator(): _clkPin(13), _fsyncPin(2), _dataPin(9), _outputonoffPin(28), _calibrationtonoffPin(26), potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND)
        {
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

        void UnMute() {
            pinMode(_outputonoffPin,   OUTPUT);
            digitalWrite(_outputonoffPin, HIGH);

            pinMode(_calibrationtonoffPin,   OUTPUT);
            digitalWrite(_calibrationtonoffPin, LOW);
        }

        void Mute() {
            pinMode(_outputonoffPin,   OUTPUT);
            digitalWrite(_outputonoffPin, LOW);

            pinMode(_calibrationtonoffPin,   OUTPUT);
            digitalWrite(_calibrationtonoffPin, LOW);
        }

        void CalibrationMode() {
            pinMode(_outputonoffPin,   OUTPUT);
            digitalWrite(_outputonoffPin, LOW);

            pinMode(_calibrationtonoffPin,   OUTPUT);
            digitalWrite(_calibrationtonoffPin, HIGH);
        }

        void UnmutedCalibrationMode() {
            pinMode(_outputonoffPin,   OUTPUT);
            digitalWrite(_outputonoffPin, HIGH);

            pinMode(_calibrationtonoffPin,   OUTPUT);
            digitalWrite(_calibrationtonoffPin, HIGH);
        }

        void ManualOutPut(uint8_t output)
        {
            setFreq(1000, 0); //ATTNUATOR OFF

            UnMute();

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

class dBMeter
{
    public:
        struct Measurement {
            bool IsSaturated(const uint16_t Saturation = 1023) {
                if (dBLeft >= Saturation || dBRight >= Saturation)
                    return true;
                return false;
            }
            Measurement() {}
            Measurement(const double dB_, const uint8_t RV_): dB(dB_), RV(RV_) {
            }
            std::string ToString() {
                char stringbuffer[255];
                char sz_dB[8];
                dtostrf(dB, 4, 1, sz_dB);

                int bitsLeft = ceil(log(dBLeft) / log(2));
                int bitsRight = ceil(log(dBRight) / log(2));
                sprintf(stringbuffer, "RV:%i dB:%s Left:%i Right:%i Bits (L/R): %i/%i " , RV, sz_dB, dBLeft, dBRight, bitsLeft, bitsRight);
                return stringbuffer;
            }
            std::string ToStringData() {
                char stringbuffer[255];
                char sz_dB[8];
                dtostrf(dB, 4, 1, sz_dB);
                sprintf(stringbuffer, "%s,%i" , sz_dB, (dBLeft + dBRight) / 2);
                return stringbuffer;
            }
            double dB;
            uint8_t RV;
            uint16_t dBLeft;
            uint16_t dBRight;
        };
        // for chip info see https://www.analog.com/en/products/ad9833.html
        // SPI code taken from https://github.com/MajicDesigns/MD_AD9833/
        const uint8_t _inputpregainPin;
        const uint8_t _calmodePin;
        AD5254_asukiaaa potentio;

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


        dBMeter(): _inputpregainPin(30), _calmodePin(26), potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND)
        {
            potentio.begin();        // start Didital potmeter

            pinMode(_inputpregainPin,   OUTPUT);
            digitalWrite(_inputpregainPin, LOW);

            pinMode(_calmodePin,   OUTPUT);
            digitalWrite(_calmodePin, HIGH);
        }
        ~dBMeter()
        {
            //Mute Output
        }

        void getdB(double startdB, uint16_t& dBLeft, uint16_t& dBRight)
        {
            //SetInPut(InPutFit64(startdB), dBLeft, dBRight);
        }

        double GetdB45RV (uint16_t AnalogRead)
        {
            std::vector<float64_t> fit6445RV(FIT64_SIZE);
        /*    
            fit6445RV[6] = fp64_atof("8.213445203525542896e-16");
            fit6445RV[5] = fp64_atof("-3.516280429832908025e-12");
            fit6445RV[4] = fp64_atof("6.220316548106305250e-09");
            fit6445RV[3] = fp64_atof("-5.824117802733188175e-06");
            fit6445RV[2] = fp64_atof("3.055317159880055911e-03");
            fit6445RV[1] = fp64_atof("-8.699673953056390463e-01");
            fit6445RV[0] = fp64_atof("1.131057223665053613e+02");
        */
            fit6445RV[6] = fp64_atof("2.380560187449659372e-15");
            fit6445RV[5] = fp64_atof("-7.737644429946968462e-12");
            fit6445RV[4] = fp64_atof("9.907876544913139965e-09");
            fit6445RV[3] = fp64_atof("-6.364225979394382238e-06");
            fit6445RV[2] = fp64_atof("2.177099620579266408e-03");
            fit6445RV[1] = fp64_atof("-4.064625658563530419e-01");
            fit6445RV[0] = fp64_atof("4.537370556334850846e+01");
            float64_t x = fp64_sd(AnalogRead);
            float64_t y = fp64_add(fit6445RV[0], fp64_mul(fit6445RV[1], x));
            for (int i = FIT_ORDER; i != 1 ; i--)
            {
                y = fp64_add(y, fp64_mul(fit6445RV[i], fp64_pow(x, fp64_sd(i))));
            }

            y = fp64_fmin(y, fp64_sd(29.2));
            y = fp64_fmax(y, fp64_sd(0));

            return atof(fp64_to_string( y, 15, 2));
        }

        void GetInPut(Measurement& m)
        {
            const uint8_t leftChannelIn(2);
            const uint8_t rightChannelIn(3);
            potentio.writeRDAC(leftChannelIn, m.RV);
            potentio.writeRDAC(rightChannelIn, m.RV);
            delay(500);

            const int CH1(A0);
            const int CH2(A1);
            m.dBLeft = analogRead(CH1);
            m.dBRight = analogRead(CH2);
        }

        void RVSweep()
        {
            SignalGenerator signalGenerator;
            signalGenerator.UnmutedCalibrationMode();

            Serial.println("Sweep RV = 44 for 0 dB to 6 dB");
            for (float d = 0.0; d < 6.0; d += 0.1) {
                signalGenerator.setFreq(1000.0, d);
                Measurement m(d, 44);
                GetInPut(m);
                Serial.println(m.ToStringData().c_str());
            }

            Serial.println("Sweep RV = 99 for 6.0 dB to 12");
            for (float d = 6.0; d < 12.0; d += .1) {
                signalGenerator.setFreq(1000.0, d);
                Measurement m(d, 99);
                GetInPut(m);
                Serial.println(m.ToStringData().c_str());
            }

            Serial.println("Sweep RV = 194 for 12.0 dB to 15.5");
            for (float d = 12.0; d < 15.5; d += .1) {
                signalGenerator.setFreq(1000.0, d);
                Measurement m(d, 194);
                GetInPut(m);
                Serial.println(m.ToStringData().c_str());
            }

            Serial.println("Sweep RV = 254 for 15.5 dB to 32");
            for (float d = 15.5; d < 32.0; d += .1) {
                signalGenerator.setFreq(1000.0, d);
                Measurement m(d, 254);
                GetInPut(m);
                Serial.println(m.ToStringData().c_str());
            }
        }

        void RVSweep45()
        {
            SignalGenerator signalGenerator;
            signalGenerator.UnmutedCalibrationMode();

            Serial.println("Sweep RV = 45");
            for (float d = 0.0; d < 32.0; d += 0.1) {
                signalGenerator.setFreq(1000.0, d);
                Measurement m(d, 45);
                GetInPut(m);
                Serial.println(m.ToStringData().c_str());
            }
        }
        void Scan()
        {
            /*
                        I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC512);


                        signalGenerator.setFreq(1000.0, 0.0);
                        GetDb();
                        signalGenerator.setFreq(1000.0, 32.0);
                        GetDb();
                        signalGenerator.setFreq(1000.0, 16.0);
                        GetDb();
                        signalGenerator.setFreq(1000.0, 32.0);
                        GetDb();
                        signalGenerator.setFreq(1000.0, 0.0);
                        GetDb();
                        signalGenerator.setFreq(1000.0, 16.0);
            */
            SignalGenerator signalGenerator;
            signalGenerator.UnmutedCalibrationMode();
            signalGenerator.setFreq(1000.0, 15.0);
            int i = 45;
            for (float d = 0.0; d < 32.1; d += .1) {
                //for (float d = 0.0; d < 32.1; d += 1.0) {
                signalGenerator.setFreq(1000.0, d);
                Measurement next(d, i);
                GetInPut(next);
                Measurement prev(next);

                while (next.RV != 255 && !next.IsSaturated()) {
                    prev = next;
                    next.RV += 1;
                    GetInPut(next);
                };
                Measurement unSaturated(prev);
                if (!next.IsSaturated()) {
                    unSaturated = next;
                }
                i = unSaturated.RV;
                Serial.println(unSaturated.ToString().c_str());

            }
        }
};



#endif // SIGNALGENERATOR_H
