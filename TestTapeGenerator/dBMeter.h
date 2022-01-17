#ifndef DBMETER_H
#define DBMETER_H

#include <AD5254_asukiaaa.h>
#include "InPutTable.h"
class dBMeter
{
    protected:
        // for chip info see https://www.analog.com/en/products/ad9833.html
        // SPI code taken from https://github.com/MajicDesigns/MD_AD9833/
        const uint8_t _inputpregainPin;
        const uint8_t _calmodePin;
        AD5254_asukiaaa potentio;

    public:
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

        uint8_t getInput(double dB)
        {
            for (int i = 0; i != 256; ++i)
            {
                if (dB > InPutTable[i]) {
                    {
                        return i;
                    }
                }
            }
        }

        void getdB(double startdB, uint16_t& dBRight, uint16_t& dBLeft)
        {
            uint8_t input(getInput(startdB));
            const uint8_t leftChannelIn(2);
            const uint8_t rightChannelIn(3);
            potentio.writeRDAC(leftChannelIn, input);  
            potentio.writeRDAC(rightChannelIn, input);           
            dBLeft = analogRead(leftChannelIn);
            dBRight = analogRead(rightChannelIn);
        }
};

#endif // DBMETER_H
