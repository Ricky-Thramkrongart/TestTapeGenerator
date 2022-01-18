#ifndef OUTPUTTABLE_H
#define OUTPUTTABLE_H

uint8_t OutPutTableFit (double dB)
{
     uint8_t rv = 0.00001020475 * pow(dB,6) - 0.000634453 * pow(dB, 5) + 0.012749614 * pow(dB, 4) - 0.058752793 * pow(dB, 3) - 0.352027967 * pow(dB, 2) - 16.60582415 * dB + 254.8871207;
     rv = std::min(rv, (uint8_t)255);
     rv = std::max(rv, (uint8_t)0);
     return rv;
}

#endif // OUTPUTTABLE_H
