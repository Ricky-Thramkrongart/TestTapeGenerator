#ifndef OUTPUTTABLE_H
#define OUTPUTTABLE_H

uint8_t OutPutTableFit (double dB)
{

    double a6 = -3.809064014720473935e-07;
    double a5 = 7.447576786624759059e-05;
    double a4 = -4.762255097120195527e-03;
    double a3 = 1.282484008085452432e-01;
    double a2 = -1.083059585213202114e+00;
    double a1 = -1.385114186899055966e+01;
    double a0 = 2.541634568978011544e+02;

    uint8_t rv = a6 * pow(dB, 6) - a5 * pow(dB, 5) + a4 * pow(dB, 4) - a3 * pow(dB, 3) - a2 * pow(dB, 2) - a1 * dB + a0;
    rv = std::min(rv, (uint8_t)255);
    rv = std::max(rv, (uint8_t)0);
    return rv;
}

#endif // OUTPUTTABLE_H
