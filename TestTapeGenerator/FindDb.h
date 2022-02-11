#pragma once

#include "dBMeter.h"
std::pair<double, double> f(SignalGenerator& signalGenerator, dBMeter& dbMeter, const std::pair<double, double> x0, const uint32_t Targetfreq, const std::pair<double, double>TargetdB)
{
    signalGenerator.setFreq(Targetfreq, x0);
    dBMeter::Measurement m;
    dbMeter.GetdB(m);
    Serial.println(SignalGenerator::String(Targetfreq, x0, 2));
    Serial.println(m.String(2).c_str());
    return { m.dBLeft - TargetdB.first, m.dBRight - TargetdB.second };
}

std::pair<double, double> g(SignalGenerator& signalGenerator, dBMeter& dbMeter, const std::pair<double, double> x0,  const uint32_t Targetfreq, const std::pair<double, double>TargetdB, const std::pair<double, double>f0, double& delta)
{
    std::pair<double, double> f1;
    do {
        delta += 0.1;
        std::pair<double, double> x_delta{ x0.first + delta,x0.second + delta };
        f1 = f(signalGenerator, dbMeter, x_delta, Targetfreq, TargetdB);

    } while (f1.first - f0.first < 0.2 || f1.second - f0.second < 0.2);

    return { (f1.first - f0.first) / delta, (f1.second - f0.second) / delta };
}


std::pair<double, double> FindDb(SignalGenerator& signalGenerator, dBMeter& dbMeter, const uint32_t Targetfreq, std::pair<double,double> TargetdB)
{
    cSF(sf_line, 41);
    System::UnmutedCalibrationMode();

    std::pair<double, double> x0, x1, f0, f1, g0;
    double e = 0.05;
    int step = 1, N = 100;

    e = 0.05;
    x0 = TargetdB;

    Serial.print("Target: "); Serial.print(Targetfreq); Serial.print(" "); Serial.print(TargetdB.first); Serial.print(" "); Serial.println(TargetdB.second);

    double delta = 0.0;
    do
    {
        f0 = f(signalGenerator, dbMeter, x0, Targetfreq, TargetdB);
        if (fabs(f0.first) <= e && fabs(f0.second) <= e) {
            return x0;
        }
        g0 = g(signalGenerator, dbMeter, x0, Targetfreq, TargetdB, f0, delta);

        if (g0.first == 0.0 || g0.second == 0.0)
        {
            Serial.println("Mathematical Error.");
            exit(0);
        }

        x1.first = x0.first - f0.first / g0.first;
        x1.second = x0.second - f0.second / g0.second;
        x0 = x1;
        step = step + 1;

        if (step > N)
        {
            Serial.println("Not Convergent.");
            exit(0);
        }
    } while (true);
}
