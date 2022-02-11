#pragma once

#include <DS3232RTC.h>
#include <SafeString.h>

class RTC_Helper
{
public:
    RTC_Helper() {
        RTC.begin();
        setSyncProvider(RTC.get);   // the function to get the time from the RTC
    }
    
    void pad(SafeString& s, const int n)
    {
        if (n < 10) {
            s.print(F("0"));
        }
        s.print(n);
    }
/*
    SafeString& ToString()
    {
        cSFA(sf_line, buffer);
        sf_line.clear();
        pad(sf_line, hour());
        sf_line.print(F(":"));
        pad(sf_line, minute());
        sf_line.print(F(":"));
        pad(sf_line, second());
        return sf_line;
    }
*/ 

    String ToString()
    {
        cSF(sf, 41);
        pad(sf, hour());
        sf.print(F(":"));
        pad(sf, minute());
        sf.print(F(":"));
        pad(sf, second());
        return sf.c_str();
    }

    String ToStringExt()
    {
        cSFA(sf, 41);
        sf = ToString().c_str();
        sf.print(RTC.temperature());
        sf.print(F(" C\xDF"));
        return sf.c_str();
    }

    void ToStringExt(Stream& out)
    {
        out.println(ToStringExt().c_str());
    }

    void printlnExt(Print& out)
    {
        out.println(ToStringExt().c_str());
    }

};

