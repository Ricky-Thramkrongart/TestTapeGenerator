#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include <AD5254_asukiaaa.h>
#include "OutPutTable.h"
class SignalGenerator
{
    protected:
        // for chip info see https://www.analog.com/en/products/ad9833.html
        // SPI code taken from https://github.com/MajicDesigns/MD_AD9833/
        const uint8_t _clkPin;
        const uint8_t _fsyncPin;
        const uint8_t _dataPin;
        AD5254_asukiaaa potentio;

    public:
        SignalGenerator(): _clkPin(13), _fsyncPin(2), _dataPin(9), potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND)
        {
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
            output = OutPutTableFit(dBdiff);
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

#endif // SIGNALGENERATOR_H
