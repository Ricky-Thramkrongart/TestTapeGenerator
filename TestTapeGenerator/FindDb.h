#pragma once

#include "dBMeter.h"

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

std::pair<double, double> f(SignalGenerator& signalGenerator, dBMeter& dbMeter, const std::pair<double, double> x0, const uint32_t Targetfreq, const std::pair<double, double>TargetdB)
{
    if (Is_dB_OutOfRange(x0))
    {
        FatalError("f: x0 out of range");
    }
    signalGenerator.setFreq(Targetfreq, x0);
    dBMeter::Measurement m;
    dbMeter.GetdB(m);
    Serial.println(SignalGenerator::String(Targetfreq, x0, 2));
    Serial.println(m.String(2).c_str());
    return { m.dBIn.first - TargetdB.first, m.dBIn.second - TargetdB.second };
}

std::pair<double, double> g(SignalGenerator& signalGenerator, dBMeter& dbMeter, const std::pair<double, double> x0, const uint32_t Targetfreq, const std::pair<double, double>TargetdB, const std::pair<double, double>f0, double& delta)
{
    std::pair<double, double> f1;
    unsigned int count = 0;
    do {
        if (Is_dB_OutOfRange(x0))
        {
            FatalError("g: x0 out of range");
        }
        std::pair<double, double> x_delta{ x0.first + delta,x0.second + delta };
        f1 = f(signalGenerator, dbMeter, x_delta, Targetfreq, TargetdB);

        if (++count > 1) {
            delta += delta;
        }
    } while (f1.first == f0.first || f1.second == f0.second);
    return { (f1.first - f0.first) / delta, (f1.second - f0.second) / delta };
}

std::pair<double, double> FindDb(SignalGenerator& signalGenerator, dBMeter& dbMeter, const uint32_t Targetfreq, std::pair<double, double> TargetdB)
{
    cSF(sf_line, 41);
    System::UnmutedCalibrationMode();

    std::pair<double, double> x0, x1, f0, f1, g0;
    int step = 1, N = 10;
    
    const auto epsilon = 0.05 + fabs(0.01 * (TargetdB.second + TargetdB.first) / DBOUT_MAX_SERVICE) + fabs(0.05 * Targetfreq / 25000.0);
    Serial.print("epsilon: "); Serial.println(epsilon);

    x0 = TargetdB;
    Serial.print("Target: "); Serial.print(Targetfreq); Serial.print(" "); Serial.print(TargetdB.first); Serial.print(" "); Serial.println(TargetdB.second);

    double delta = epsilon;
    do
    {
        f0 = f(signalGenerator, dbMeter, x0, Targetfreq, TargetdB);
        if (fabs(f0.first) <= epsilon && fabs(f0.second) <= epsilon) {
            return x0;
        }

        g0 = g(signalGenerator, dbMeter, x0, Targetfreq, TargetdB, f0, delta);
        if (g0.first == 0.0 || g0.second == 0.0)
        {
            FatalError("g0 is invalid (0.0)");
        }

        x1.first = x0.first - f0.first / g0.first;
        x1.second = x0.second - f0.second / g0.second;
        x0 = x1;
        step = step + 1;

        if (step > N)
        {
            FatalError("f is not convergent");
        }
    } while (true);
}
