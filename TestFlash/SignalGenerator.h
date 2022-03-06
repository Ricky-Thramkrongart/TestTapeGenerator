#pragma once

#include <AD5254_asukiaaa.h>
#include <fp64lib.h>

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
    static SignalGenerator& Get() 
    {
        static SignalGenerator* signalGenerator = new SignalGenerator();
        return *signalGenerator;
    }


    uint16_t OutPutFit64(const double dB)
    {
        float64_t dB64 = fp64_sd(dB);
        float64_t dB64_pow = fp64_sd(1.0);

        float64_t rv = fp64_add(fit64[0], fp64_mul(fit64[1], dB64));
        for (int i = 2; i != fit64.size(); ++i)
        {
            dB64_pow = fp64_mul(dB64, dB64_pow);
            rv = fp64_add(rv, fp64_mul(fit64[i], dB64_pow));
        }

        //rv = fp64_round(rv);
        rv = fp64_fmin(rv, fp64_sd(255));
        rv = fp64_fmax(rv, fp64_sd(0));
        return fp64_to_uint16(rv);
    }

    void begin(void) {
        if (fit64.empty()) {
            Serial.print("fit64.empty()");
            delay(1000);
            exit(EXIT_FAILURE);
        }

        //ReadFit64FromEEPROM();

        potentio.begin();        // start Didital potmeter

        pinMode(_clkPin, OUTPUT);
        digitalWrite(_clkPin, LOW);

        pinMode(_fsyncPin, OUTPUT);
        digitalWrite(_fsyncPin, HIGH);

        pinMode(_dataPin, OUTPUT);
    }

    SignalGenerator() : potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND)
    {
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

    void setdB(const std::pair<double, double>& dB)
    {

        double dBdiff;
        const double dBMax(std::max(dB.first, dB.second));
        if (dBMax > -10) {
            digitalWrite(10, LOW);  // -10 db att OFF
            digitalWrite(7, LOW);  // -20 db att OFF
            dBdiff = 0;
        }
        else if (dBMax <= -10 && dBMax > -20) {
            digitalWrite(10, HIGH);  // -10 db att ON
            digitalWrite(7, LOW);  // -20 db att ON
            dBdiff = 10;  // beregn rest att fra digi-pot
        }
        else if (dBMax <= -20 && dBMax > -30) {
            digitalWrite(10, LOW); // -10 db att OFF
            digitalWrite(7, HIGH);   // -20 db att ON
            dBdiff = 20; // beregn rest att fra digi-pot
        }
        else if (dBMax <= -30) {
            digitalWrite(10, HIGH); // -10 db att ON
            digitalWrite(7, HIGH);  // -20 db att ON
            dBdiff = 30; // beregn rest att fra digi-pot
        }

        std::pair<uint8_t, uint8_t>output{ OutPutFit64(dB.first + dBdiff) ,OutPutFit64(dB.second + dBdiff) };

        //cSF(sf_line, 128);
        //sf_line.print(F("dBLeft: ")); sf_line.print(dB.first, 1, 5); sf_line.print(F("dBRight: ")); sf_line.print(dB.second, 1, 5); sf_line.print(F(" dBDiff: ")); sf_line.print(dBdiff, 1, 5); sf_line.print(F(" outputLeft: ")); sf_line.print(output.first); sf_line.print(F(" outputRight: ")); sf_line.print(output.second);
        //Serial.println(sf_line);

        const uint8_t leftChannelOut(1);
        const uint8_t rightChannelOut(0);
        potentio.writeRDAC(leftChannelOut, output.first);
        potentio.writeRDAC(rightChannelOut, output.second);
        delay(200);
    }

    void setFreq(const uint32_t f, const std::pair<double, double>& dB)
    {
        //Serial.println(String(f, dB, 2));
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
};

SignalGenerator signalGenerator;
