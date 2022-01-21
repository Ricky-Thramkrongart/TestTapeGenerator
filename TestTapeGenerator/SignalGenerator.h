#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include <AD5254_asukiaaa.h>
#include <fp64lib.h>
#include <EEPROM.h>
#include <ArduinoSTL.h>
#include <algorithm>
#include <iterator>

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
    protected:
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

        void getdB(double startdB, uint16_t& dBRight, uint16_t& dBLeft)
        {

            uint8_t input(InPutFit64(startdB));
            const uint8_t leftChannelIn(2);
            const uint8_t rightChannelIn(3);
            potentio.writeRDAC(leftChannelIn, input);
            potentio.writeRDAC(rightChannelIn, input);
            dBLeft = analogRead(leftChannelIn);
            dBRight = analogRead(rightChannelIn);
        }

        void ManualInPut(uint8_t input, int& dBLeft, int& dBRight)
        {
            //setFreq(1000, 0); //ATTNUATOR OFF

            ///UnMute();

            const uint8_t leftChannelIn(2);
            const uint8_t rightChannelIn(3);
            potentio.writeRDAC(leftChannelIn, input);  //Right
            potentio.writeRDAC(rightChannelIn, input);  //Left

            const int CH1(A0);
            const int CH2(A1);
            dBLeft = analogRead(CH1);
            dBRight = analogRead(CH2);
        }

        void Scan()
        {
            SignalGenerator signalGenerator;
            signalGenerator.UnmutedCalibrationMode();
            signalGenerator.setFreq(1000.0, 0.0);
            float d = 0.0;
            for (int i = 0; i != 256; i++) {
                int dBLeft;
                int dBRight;
                ManualInPut(i, dBLeft, dBRight);

                char stringbuffer[255];
                char sz_d[8];
                dtostrf(d, 4, 1, sz_d);
                sprintf(stringbuffer, "%i %s %i %i" , i, sz_d, dBLeft, dBRight);
                Serial.println(stringbuffer);
            }
        }


};



#endif // SIGNALGENERATOR_H
