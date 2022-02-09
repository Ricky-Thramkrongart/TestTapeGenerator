#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include <AD5254_asukiaaa.h>
#include <fp64lib.h>
#include <EEPROM.h>
#include <ArduinoSTL.h>
#include <algorithm>
#include <iterator>
#include <I2C_eeprom.h>
#include "Relay.h"
#include "System.h"
#include "PolyVal.h"

class SignalGenerator
{
private:
    SignalGenerator(const SignalGenerator&) = delete;
    SignalGenerator& operator=(const SignalGenerator&) = delete;
protected:
    // for chip info see https://www.analog.com/en/products/ad9833.html
    // SPI code taken from https://github.com/MajicDesigns/MD_AD9833/
    static constexpr auto _clkPin = 13;
    static constexpr auto _fsyncPin = 2;
    static constexpr auto _dataPin = 9;
    AD5254_asukiaaa potentio;

public:
    static std::vector<float64_t> fit64;
    void WriteFit64ToEEPROM(void)
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
    void ReadFit64FromEEPROM(void)
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

    uint16_t OutPutFit64(double dB)
    {
        float64_t dB64 = fp64_sd(dB);
        float64_t rv = fp64_add(fit64[0], fp64_mul(fit64[1], dB64));
        for (int i = fit64.size() - 1; i != 1; i--)
        {
            rv = fp64_add(rv, fp64_mul(fit64[i], fp64_pow(dB64, fp64_sd(i))));
        }

        //rv = fp64_round(rv);
        rv = fp64_fmin(rv, fp64_sd(255));
        rv = fp64_fmax(rv, fp64_sd(0));
        return atoi(fp64_to_string(rv, 15, 2));
    }


    void Device1()
    {
        const char* fit64_flashTable[] PROGMEM = {
            "-6.162314822380227e-08",
            "-4.378308439972472e-06",
            "-0.00011688793870179859",
            "-0.0013040839584018702",
            "-0.003870823753524697",
            "-0.020528115676117634",
            "-0.5461083074144886",
            "16.406533912931025",
            "254.83884893125213"
        };
        constexpr auto fit64_size = sizeof(fit64_flashTable) / sizeof(const char*);
        fit64 = std::vector <float64_t>(fit64_size);
        for (int i = 0; i != fit64.size(); i++) {
            fit64[fit64_size - i - 1] = fp64_atof(const_cast<char*>(fit64_flashTable[i]));
        }
    }

    void Device2()
    {
        const char* fit64_flashTable[] PROGMEM = {
            "2.6008901174857894e-05",
            "0.001382974224325812",
            "0.025824261833570294",
            "0.1628642044710132",
            "0.05232081299714197",
            "17.632868585802612",
            "255.28888529165388"
        };
        constexpr auto fit64_size = sizeof(fit64_flashTable) / sizeof(const char*);
        fit64 = std::vector <float64_t>(fit64_size);
        for (int i = 0; i != fit64.size(); i++) {
            fit64[fit64_size - i - 1] = fp64_atof(const_cast<char*>(fit64_flashTable[i]));
        }
    }

    SignalGenerator() : potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND)
    {
        System::UnMute();

        //ReadFit64FromEEPROM();
        Device1();

        potentio.begin();        // start Didital potmeter

        pinMode(_clkPin, OUTPUT);
        digitalWrite(_clkPin, LOW);

        pinMode(_fsyncPin, OUTPUT);
        digitalWrite(_fsyncPin, HIGH);

        pinMode(_dataPin, OUTPUT);
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
        if (dB > -5) {
            digitalWrite(7, LOW);  // -5 db att OFF
            digitalWrite(10, LOW);  // -10 db att OFF
            dBdiff = dB;
        }
        else if (dB <= -5 && dB > -10) {
            digitalWrite(7, HIGH);  // -5 db att ON
            digitalWrite(10, LOW);  // -10 db att OFF
            dBdiff = dB + 5;  // beregn rest att fra digi-pot
        }
        else if (dB <= -10 && dB > -15) {
            digitalWrite(7, LOW);   // -5 db att OFF
            digitalWrite(10, HIGH); // -10 db att ON
            dBdiff = dB + 10; // beregn rest att fra digi-pot
        }
        else if (dB <= -15) {
            digitalWrite(7, HIGH);  // -5 db att ON
            digitalWrite(10, HIGH); // -10 db att ON
            dBdiff = dB + 15; // beregn rest att fra digi-pot
        }
        output = OutPutFit64(dBdiff);
        //cSF(sf_line, 41);
        //sf_line.print(F("dBm: ")); sf_line.print(dB, 1, 5); sf_line.print(F(" dBDiff: ")); sf_line.print(dBdiff, 1, 5); sf_line.print(" Output: "); sf_line.print(output);
        //Serial.println(sf_line);

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

        const uint16_t b28 = (1UL << 13);
        const uint16_t freq = (1UL << 14);

        const double   f_clk = 25e6;
        const double   scale = 1UL << 28;
        const uint32_t n_reg = f * scale / f_clk;

        const uint16_t f_low = n_reg & 0x3fffUL;
        const uint16_t f_high = (n_reg >> 14) & 0x3fffUL;

        spiSend(b28);
        spiSend(f_low | freq);
        spiSend(f_high | freq);
    }
    static String String(double f, double dB, uint8_t decs = 1)
    {
        cSF(sf_line, 41);
        sf_line.print(F("Generator: "));
        sf_line.print(f, 0, 5);
        sf_line.print(F("Hz "));
        sf_line.print(dB, decs, 5);
        sf_line.print(F("dBm"));
        return sf_line.c_str();
    }

};

std::vector<float64_t> SignalGenerator::fit64;

#endif // SIGNALGENERATOR_H
