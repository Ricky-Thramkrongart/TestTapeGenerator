#pragma once

#include <fp64lib.h>

double PolyVal(const std::vector <float64_t>& fit64, uint16_t v, double offset)
{
    float64_t x = fp64_sd(v);
    float64_t x_pow = fp64_sd(1.0);
    float64_t y = fit64[0];
    for (int i = 1; i != fit64.size(); ++i)
    {
        x_pow = fp64_mul(x, x_pow);
        y = fp64_add(y, fp64_mul(fit64[i], x_pow));
    }

    y = fp64_fmax(y, fp64_sd(-29.2));
    y = fp64_fmin(y, fp64_sd(0));

    return fp64_ds(y) + offset;
}