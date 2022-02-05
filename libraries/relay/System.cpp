#include <System.h>
    static void System::UnMute()
    {
        muteRelay.Disable();
        calibrationRelay.Disable();
    }

    static void System::Mute()
    {
        muteRelay.Enable();
        calibrationRelay.Disable();
    }

    static void System::CalibrationMode()
    {
        muteRelay.Enable();
        calibrationRelay.Enable();
    }

    static void System::UnmutedCalibrationMode()
    {
        muteRelay.Disable();
        calibrationRelay.Enable();
    }

    static bool System::GetCalibration()
    {
        return  calibrationRelay.IsEnabled();
    }
    static bool System::GetMute()
    {
        return  muteRelay.IsEnabled();
    }


Relay System::muteRelay(Relay(28, true));
Relay System::calibrationRelay(Relay(26));