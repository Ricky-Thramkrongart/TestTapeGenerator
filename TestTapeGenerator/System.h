#pragma once

class System
{
private:
    //Disables copy constructor
    System(const System&) = delete;
    System(System&) = delete;
    //Disables assignment operator
    System& operator=(const System&) = delete;
    System& operator=(System&) = delete;

protected:
    static Relay muteRelay;
    static Relay calibrationRelay;
public:
    static void UnMute()
    {
        muteRelay.Disable();
        calibrationRelay.Disable();
    }

    static void Mute()
    {
        muteRelay.Enable();
        calibrationRelay.Disable();
    }

    static void CalibrationMode()
    {
        muteRelay.Enable();
        calibrationRelay.Enable();
    }

    static void UnmutedCalibrationMode()
    {
        muteRelay.Disable();
        calibrationRelay.Enable();
    }

    static bool GetCalibration()
    {
        return  calibrationRelay.IsEnabled();
    }
    static bool GetMute()
    {
        return  muteRelay.IsEnabled();
    }
};
Relay System::muteRelay(Relay(28, true));
Relay System::calibrationRelay(Relay(26));