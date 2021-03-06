#pragma once

#include "dBMeter.h"
#include <assert.h>

void FatalError(const char* Msg)
{
    LCD_Helper lcdHelper;
    lcdHelper.Line(0, Msg);
    lcdHelper.Line(1, "!! System Halted !!");
    lcdHelper.Show(Serial);
    lcdHelper.lcd.blink_on();
    lcdHelper.Show(1000);
    exit(EXIT_FAILURE);
}

bool clearBestGuess;
dBMeter::Measurement m0;

void SaveBestdB(const dBMeter::Measurement& m, const std::pair<double, double>TargetdB)
{
    if (clearBestGuess) {
        m0 = m;
        clearBestGuess = false;
        return;
    }

    if (fabs(m0.dBIn.first - TargetdB.first) > fabs(m.dBIn.first - TargetdB.first)) {
        m0.dBIn.first = m.dBIn.first;
        m0.dBOut.first = m.dBOut.first;
        m0.Raw.first = m.Raw.first;
        m0.Std.first = m.Std.first;
    }

    if (fabs(m0.dBIn.second - TargetdB.second) > fabs(m.dBIn.second - TargetdB.second)) {
        m0.dBIn.second = m.dBIn.second;
        m0.dBOut.second = m.dBOut.second;
        m0.Raw.second = m.Raw.second;
        m0.Std.second = m.Std.second;
    }
}

std::pair<double, double> f(const std::pair<double, double> x0, const uint32_t Targetfreq, const std::pair<double, double>TargetdB)
{
    SignalGenerator::Get().setFreq(Targetfreq, x0);
    dBMeter::Measurement m(x0);
    dBMeter::Get().GetdB(m);
    SaveBestdB(m, TargetdB);
    //Serial.println(SignalGenerator::String(Targetfreq, x0, 1));
    //Serial.println(m.String(1).c_str());
    return { m.dBIn.first - TargetdB.first, m.dBIn.second - TargetdB.second };
}

std::pair<double, double> g(const std::pair<double, double> x0, const uint32_t Targetfreq, const std::pair<double, double>TargetdB, const std::pair<double, double>f0, double& delta)
{
    std::pair<double, double> f1;
    unsigned int count = 0;
    std::pair<double, double> g0;
    do {
        std::pair<double, double> x_delta{ x0.first + delta,x0.second + delta };
        if (Is_dBOut_OutOfRange(x_delta)) {
            return { 0.0, 0.0 };
        }
        f1 = f(x_delta, Targetfreq, TargetdB);
        if (++count > 1) {

            delta += delta;
        }
        //cSF(sf_line, 41);
        //sf_line.print(F("g0: count: ")); sf_line.print(count); sf_line.print(F(" delta: ")); sf_line.print(delta);
        //Serial.println(sf_line);

        g0 = { (f1.first - f0.first) / delta, (f1.second - f0.second) / delta };
    } while (g0.first <= 0.0 || g0.second <= 0.0);
    return g0;
}

std::pair<double, double> FinddB(const uint32_t Targetfreq, std::pair<double, double> TargetdB, std::pair<double, double> StartGuess, LCD_Helper& lcdHelper)
{
    dBMeter::Get().Cabling();

    std::pair<double, double> x0, x1, f0, f1, g0;
    
    double epsilon = 0.05 + fabs(0.01 * (TargetdB.second + TargetdB.first) / DBOUT_MAX_SERVICE) + fabs(0.1 * Targetfreq / 25000.0);

    clearBestGuess = true;

    for (int i = 1;  i != 20; ++i)
    {
        x0 = StartGuess;
        epsilon += 0.05;
        double delta = 4*epsilon;

        cSF(sf_line, 41);
        sf_line.print(F("-> ")); sf_line.print(Targetfreq,0,5); sf_line.print(F("Hz ")); sf_line.print(TargetdB.first,1,5); sf_line.print(F("dBm ")); sf_line.print(TargetdB.second, 1, 5); sf_line.print(F("dBm e: ")); sf_line.print(epsilon,2);
        lcdHelper.Line(1, sf_line);
        lcdHelper.Show(Serial);
        lcdHelper.Show();

        do
        {
            //Serial.print(F("x0:")); Serial.print(x0.first); Serial.print(F(" ")); Serial.println(x0.second);

            f0 = f(x0, Targetfreq, TargetdB);
            if (fabs(m0.dBIn.first - TargetdB.first) <= epsilon && fabs(m0.dBIn.second - TargetdB.second) <= epsilon) {
                return m0.dBOut;
            }
            f0 = { m0.dBIn.first - TargetdB.first,  m0.dBIn.second - TargetdB.second };
            x0 = m0.dBOut;

            g0 = g(x0, Targetfreq, TargetdB, f0, delta);
            if (fabs(m0.dBIn.first - TargetdB.first) <= epsilon && fabs(m0.dBIn.second - TargetdB.second) <= epsilon) {
                return m0.dBOut;
            }
            f0 = { m0.dBIn.first - TargetdB.first,  m0.dBIn.second - TargetdB.second };
            x0 = m0.dBOut;

            if (g0.first == 0.0 || g0.second == 0.0)
            {
                Serial.println(F("G0 = 0"));
                break;
            }
            
            //Serial.print(F("x0:")); Serial.print(x0.first); Serial.print(F(" ")); Serial.println(x0.second);

            x1.first = x0.first - f0.first / g0.first;
            x1.second = x0.second - f0.second / g0.second;
            x0 = x1;

            x0.first = std::max(x0.first, (double)DBOUT_MIN);
            x0.second = std::max(x0.second, (double)DBOUT_MIN);
            x0.first = std::min(x0.first, (double)DBOUT_MAX);
            x0.second = std::min(x0.second, (double)DBOUT_MAX);

            //Serial.print(F("x0:")); Serial.print(x0.first); Serial.print(F(" ")); Serial.println(x0.second);
        } while (!Is_dBOut_OutOfRange(x0));
    }
    FatalError("f is not convergent");
}
