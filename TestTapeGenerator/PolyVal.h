#pragma once

#include <fp64lib.h>
#include <ArduinoSTL.h>

double PolyVal(const std::vector <float64_t>& fit64, uint16_t v)
{
    float64_t x = fp64_sd(v);
    float64_t y = fp64_add(fit64[0], fp64_mul(fit64[1], x));
    for (int i = fit64.size() - 1; i != 1; i--)
    {
        y = fp64_add(y, fp64_mul(fit64[i], fp64_pow(x, fp64_sd(i))));
    }

    y = fp64_fmax(y, fp64_sd(-29.2));
    y = fp64_fmin(y, fp64_sd(0));

    return atof(fp64_to_string(y, 15, 2));
}