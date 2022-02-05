#pragma once

#include <Relay.h>

class System
{
private:
    System(const System&) = delete;
    System& operator=(const System&) = delete;
protected:
    static Relay muteRelay;
    static Relay calibrationRelay;
public:
    static void UnMute();
    static void Mute();
    static void CalibrationMode();
    static void UnmutedCalibrationMode();
    static bool GetCalibration();
    static bool GetMute();
};

