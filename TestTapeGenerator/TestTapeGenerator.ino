#include "System.h"
#include "Menu.h"
#include "Printer.h"
#include "Controls.h"
#include "Dialog.h"
#include "RTCHelper.h"
#include <ArduinoSTL.h>
#include <bitset>
#include <ArxSmartPtr.h>
#include <EEPROM.h>
#include <DS3232RTC.h>      // https://github.com/JChristensen/DS3232RTC
#include <Regexp.h>         // https://github.com/nickgammon/Regexp/
#include <MemoryFree.h>
#include "Splash.h"
#include "SelfTest.h"
#include "NewTestTape.h"
#include "Tools.h"

using namespace std;


class PotentioMeterOutputSelection : public Menu
{
public:
    PotentioMeterOutputSelection() : Menu(256) {}
    void FullUpdate() {
        cSF(sf_line, 41);
        sf_line.print(F("Potention Meter Output: "));
        sf_line.print(Current);
        SignalGenerator::Get().ManualOutPut(Current);
        lcdhelper.Line(0, sf_line);
    }
};

class MainMenu : public Menu
{
public:
    MainMenu() : Menu(10) {}
    void FullUpdate() {
        const __FlashStringHelper* str = 0;
        switch (Current) {
        case 0:
            str = F("Create Test Tape");
            break;
        case 1:
            str = F("Create Sweep Tape");
            break;
        case 2:
            str = F("Print Label");
            break;
        case 3:
            str = F("Tests...");
            break;
        case 4:
            str = F("Start Signal Generator");
            break;
        case 5:
            str = F("Start dBMeter");
            break;
        case 6:
            str = F("Output Hardware Calibration");
            break;
        case 7:
            str = F("Input Hardware Calibration");
            break;
        case 8:
            str = F("dBMeter RV Scan");
            break;
        case 9:
            str = F("Set Time");
            break;
        }
        lcdhelper.Line(0, F("== Main Menu ====================="));
        lcdhelper.Line(1, str);
    }
};

void OutputHardwareCalibration(void)
{
    PotentioMeterOutputSelection().Execute();
}

void dBMeterScan(void)
{
    dBMeter::Get().Scan();
}

void InputHardwareCalibration(void)
{
    dBMeter::Get().RVSweep();
}

template <class T, size_t N>
size_t array_size(T(&array)[N]) {
    return N;
}

void setup()
{
    Serial.begin(115200);
    splashscreen();
    System::OutPutOff();
    System::SetupDevice();

    Serial.setTimeout(500);
    Serial.println("Prompt>");
    String str(Serial.readString());
    str.trim();
    if (str.length()) {
        MatchState ms(const_cast<char*>(str.c_str()));
        if (REGEXP_MATCHED == ms.Match("RVSweep"))
            InputHardwareCalibration();
    }

    do {
        MainMenu mainMenu;
        if (mainMenu.Execute() == ButtonPanel<BasePanel>::IDOK) {
            switch (mainMenu.Current)
            {
            case 0:
                NewTestTape(); 
                break;
            case 1:
                TestTapeSweep();
                break;
            case 2:
                PrintLabel();
                break;
            case 3:
                Tests();
                break;
            case 4:
                SignalGeneratorOkDialog().Execute();
                break;
            case 5:
                dBMeterOkDialog().Execute();
                break;
            case 6:
                OutputHardwareCalibration();
                break;
            case 7:
                InputHardwareCalibration();
                break;
            case 8:
                dBMeterScan();
                break;
            case 9:
                SetDateTime();
                break;

            };
        }
    } while (1);
}

void loop() {
}
