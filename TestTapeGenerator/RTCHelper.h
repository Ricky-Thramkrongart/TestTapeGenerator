#pragma once

#include <DS3232RTC.h>

class RTC_Helper
{
public:
    RTC_Helper() {
        RTC.begin();
        setSyncProvider(RTC.get);   // the function to get the time from the RTC
    }
    std::string ToString() {
        char stringbuffer[255];
        sprintf(stringbuffer, "%02i:%02i:%02i", hour(), minute(), second());
        return stringbuffer;
    }
    std::string ToStringExt() {
        char stringbuffer[255];
        char degree = '\xDF';
        sprintf(stringbuffer, "%02i:%02i:%02i %02i C", hour(), minute(), second(), RTC.temperature());
        stringbuffer[11] = degree;
        return stringbuffer;
    }
};