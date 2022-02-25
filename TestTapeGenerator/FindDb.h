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

std::pair<double, double> f(const std::pair<double, double> x0, const uint32_t Targetfreq, const std::pair<double, double>TargetdB)
{
    SignalGenerator::Get().setFreq(Targetfreq, x0);
    dBMeter::Measurement m;
    dBMeter::Get().GetdB(m);
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
        g0 = { (f1.first - f0.first) / delta, (f1.second - f0.second) / delta };
    } while (g0.first <= 0.0 || g0.second <= 0.0);
    return g0;
}

std::pair<double, double> FinddB(const uint32_t Targetfreq, std::pair<double, double> TargetdB, std::pair<double, double> StartGuess, double& epsilon, LCD_Helper& lcdHelper)
{
    dBMeter::Get().Cabling();

    std::pair<double, double> x0, x1, f0, f1, g0;
    
    if (epsilon == 0.0) {
        epsilon = 0.05 + fabs(0.01 * (TargetdB.second + TargetdB.first) / DBOUT_MAX_SERVICE) + fabs(0.1 * Targetfreq / 25000.0);
    }

    for (int i = 1;  i != 10; ++i)
    {
        x0 = StartGuess;
        epsilon += 0.05;
        double delta = 8 * epsilon;

        cSF(sf_line, 41);
        sf_line.print("-> "); sf_line.print(Targetfreq,0,5); sf_line.print("Hz "); sf_line.print(TargetdB.first,1,5); sf_line.print("dBm "); sf_line.print(TargetdB.second, 1, 5); sf_line.print("dBm e: "); sf_line.print(epsilon,2);
        lcdHelper.Line(1, sf_line);
        lcdHelper.Show(Serial);
        lcdHelper.Show();

        do
        {
            f0 = f(x0, Targetfreq, TargetdB);
            if (fabs(f0.first) <= epsilon && fabs(f0.second) <= epsilon) {
                return x0;
            }

            g0 = g(x0, Targetfreq, TargetdB, f0, delta);
            if (g0.first == 0.0 || g0.second == 0.0)
            {
                break;
            }

            x1.first = x0.first - f0.first / g0.first;
            x1.second = x0.second - f0.second / g0.second;
            x0 = x1;
        } while (!Is_dBOut_OutOfRange(x0));
    }
    FatalError("f is not convergent");
}
