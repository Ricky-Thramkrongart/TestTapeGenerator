#pragma once

#include "dBMeter.h"
double f(SignalGenerator& signalGenerator, dBMeter& dbMeter, const double x0, double const Target)
{
    signalGenerator.setFreq(1000, x0, x0);
    dBMeter::Measurement m;
    dbMeter.GetdB(m);
    Serial.println(SignalGenerator::String(1000, x0, 2));
    Serial.println(m.String(2).c_str());
    return m.dBLeft - Target;
}

double g(SignalGenerator& signalGenerator, dBMeter& dbMeter, double x0, const double Target, const double f0, double& delta)
{
    double f1;
    do {
        delta += 0.1;
        f1 = f(signalGenerator, dbMeter, x0 + delta, Target);
    } while (f1 - f0 < 0.2);
    return (f1 - f0) / delta;
}

double FindDb(SignalGenerator& signalGenerator, dBMeter& dbMeter, double Target)
{
    cSF(sf_line, 41);
    System::UnmutedCalibrationMode();

    double x0, x1, f0, f1, g0, e;
    int step = 1, N = 100;

    e = 0.05;
    x0 = Target;

    Serial.print("Target: "); Serial.println(Target);

    double delta = 0.0;

    do
    {
        f0 = f(signalGenerator, dbMeter, x0, Target);
        if (fabs(f0) <= e) {
            return x0;
        }
        g0 = g(signalGenerator, dbMeter, x0, Target, f0, delta);

        if (g0 == 0.0)
        {
            Serial.println("Mathematical Error.");
            exit(0);
        }

        x1 = x0 - f0 / g0;
        x0 = x1;
        step = step + 1;

        if (step > N)
        {
            Serial.println("Not Convergent.");
            exit(0);
        }
    } while (true);
}
