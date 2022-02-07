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

using namespace std;


class PotentioMeterOutputSelection : public Menu
{
protected:
    SignalGenerator signalGenerator;
public:
    PotentioMeterOutputSelection() : Menu(256) {}
    void FullUpdate() {
        cSF(sf_line, 41);
        sf_line.print(F("Potention Meter Output: "));
        sf_line.print(Current);
        signalGenerator.ManualOutPut(Current);
        lcdhelper.Line(0, sf_line);
    }
};

class MainMenu : public Menu
{
public:
    MainMenu() : Menu(8) {}
    void FullUpdate() {
        const __FlashStringHelper* str = 0;
        switch (Current) {
        case 0:
            str = F("Create Test Tape");
            break;
        case 1:
            str = F("Set Time");
            break;
        case 2:
            str = F("Start Signal Generator");
            break;
        case 3:
            str = F("Start dBMeter");
            break;
        case 4:
            str = F("Output Hardware Calibration");
            break;
        case 5:
            str = F("Output Poly Fit");
            break;
        case 6:
            str = F("Input Hardware Calibration");
            break;
        case 7:
            str = F("dBMeter RV Scan");
            break;
        }
        lcdhelper.Line(0, F("== Main Menu =================="));
        lcdhelper.Line(1, str);
    }
};

void SetDateTime()
{
    LCD_Helper lcdhelper;
    RTC_Helper rtchelper;
    lcdhelper.Line(0, F("Reading Date Time from Serial Port"));
    lcdhelper.Line(1, F("Format: [yyyy/mm/dd HH.MM.SS] 115200 Baud"));
    lcdhelper.Show();
    Serial.setTimeout(500);
    while (Serial.available() > 0) Serial.read();
    Serial.flush();
    lcdhelper.Show(Serial);
    do {
        if (Serial.available() >= 19) {
            MatchState ms;
            String str(Serial.readString());
            ms.Target(str.c_str());
            char result = ms.Match("(%d%d%d%d)/(%d%d)/(%d%d) (%d%d)\.(%d%d)\.(%d%d)");
            char cap[256];
            if (result == REGEXP_MATCHED)
            {
                time_t t;
                tmElements_t tm;
                int index = 0;
                ms.GetCapture(cap, index++);
                uint16_t y = atoi(cap);
                tm.Year = CalendarYrToTm(atoi(cap));
                ms.GetCapture(cap, index++);
                tm.Month = atoi(cap);
                ms.GetCapture(cap, index++);
                tm.Day = atoi(cap);
                ms.GetCapture(cap, index++);
                tm.Hour = atoi(cap);
                ms.GetCapture(cap, index++);
                tm.Minute = atoi(cap);
                ms.GetCapture(cap, index++);
                tm.Second = atoi(cap);
                t = makeTime(tm);
                RTC.set(t);        // use the time_t value to ensure correct weekday is set
                setSyncProvider(RTC.get);   // the function to get the time from the RTC
                lcdhelper.Line(2, F("Recieved Date Time."));
                lcdhelper.Show();
                lcdhelper.Show(Serial);
                delay(2000);
                return;
            }
            while (Serial.available() > 0) Serial.read();
        }
    } while (true);
}

void OutputHardwareCalibration(void)
{
    PotentioMeterOutputSelection().Execute();
}

void dBMeterScan(void)
{
    dBMeter dbMeter;
    dbMeter.Scan();
}

void StartSignalGenerator()
{

    LCD_Helper lcdhelper;
    lcdhelper.Line(0, F("Signal Generator"));
    lcdhelper.Line(1, F("Format: [float float] 115200 Baud"));

    lcdhelper.Show();

    Serial.setTimeout(500);
    while (Serial.available() > 0) Serial.read();
    Serial.flush();
    lcdhelper.Show(Serial);
    SignalGenerator signalGenerator;
    System::UnmutedCalibrationMode();
    dBMeter dbMeter;

    do {
        if (Serial.available()) {
            MatchState ms;
            String str(Serial.readString());
            ms.Target(str.c_str());
            char result = ms.Match("([-+]?[0-9]*\.?[0-9]+) ([-+]?[0-9]*\.?[0-9]+)");
            char cap[256];

            if (result == REGEXP_MATCHED)
            {
                int index = 0;
                ms.GetCapture(cap, index++);
                double freq = atof(cap);
                ms.GetCapture(cap, index++);
                double dB = atof(cap);
                signalGenerator.setFreq(freq, dB);
                dBMeter::Measurement m;
                m.dB = dB;
                double dBLeft, dBRight;
                dbMeter.GetdB(m, dBLeft, dBRight);
                lcdhelper.Line(2, SignalGenerator::String(freq, dB));
                lcdhelper.Line(3, m.String());
                lcdhelper.Show();
                lcdhelper.Show(Serial);
            }
            while (Serial.available() > 0) Serial.read();
        }
        delay(1000);

    } while (true);
}

void StartdBMeter()
{
    LCD_Helper lcdhelper;
    lcdhelper.Line(0, F("dBMeter"));
    lcdhelper.Show();
    lcdhelper.Show(Serial);
    System::Mute();
    dBMeter dbMeter;
    do {
        dBMeter::Measurement m;
        double dBLeft, dBRight;
        dbMeter.GetdB(m, dBLeft, dBRight);
        lcdhelper.Line(3, m.String());
        lcdhelper.Show();
        lcdhelper.Show(Serial);
        delay(1000);
    } while (true);
}

void SetOutPutFit()
{
    LCD_Helper lcdhelper;
    lcdhelper.Line(0, F("Reading OutPut fit a6..a0"));
    lcdhelper.Line(1, F("Format: [sci.not.] 115200 Baud"));
    lcdhelper.Show();
    Serial.setTimeout(500);
    while (Serial.available() > 0) Serial.read();
    Serial.flush();
    lcdhelper.Show(Serial);
    SignalGenerator signalGenerator;
    int i = FIT_ORDER;
    std::vector<float64_t> fit64(FIT64_SIZE);

    do {

        if (Serial.available()) {
            String str(Serial.readString());
            MatchState ms;

            ms.Target(str.c_str());
            char result = ms.Match("([+%-]?%d+%.?%d*[eE+%-]*%d?%d?)");
            char cap[256];
            if (result == REGEXP_MATCHED && ms.level == 1)
            {
                int index = 0;
                ms.GetCapture(cap, index++);
                fit64[i] = fp64_atof(str.c_str());
                cSF(sf_Line, 100);
                sf_Line = F("Recieved a");
                sf_Line.print(i--);
                sf_Line.print(F(": "));
                sf_Line.print(fp64_to_string(fit64[i], 30, 2));
                lcdhelper.Line(2, sf_Line);
                lcdhelper.Show();
                lcdhelper.Show(Serial);
                if (i == -1) {
                    break;
                }
            }

            while (Serial.available() > 0) Serial.read();
            delay(100);
        }
        delay(1000);
    } while (true);

    signalGenerator.fit64 = fit64;
    signalGenerator.WriteFit64ToEEPROM();
    signalGenerator.ReadFit64FromEEPROM();

    for (double d = 16.0; d > -0.1; d -= 0.1) {
        cSF(sf_line, 41);
        sf_line.print(d, 4, 1);
        sf_line.print(F(" "));
        sf_line.print(signalGenerator.OutPutFit64(d));
        Serial.println(sf_line.c_str());
        delay(50);
    }
}

void InputHardwareCalibration(void)
{
    dBMeter dbMeter;
    dbMeter.RVSweep();
}

void setup()
{
    Serial.begin(115200);
    splashscreen();
    selftest();

    Serial.setTimeout(500);
    Serial.println("Prompt>");
    String str(Serial.readString());
    str.trim();
    if (str.length()) {
        MatchState ms(str.c_str());
        if (REGEXP_MATCHED == ms.Match("RVSweep"))
            InputHardwareCalibration();
    }

    do {
        MainMenu mainMenu;
        if (mainMenu.Execute()) {
            switch (mainMenu.Current)
            {
            case 0:
                NewTestTape();
                break;
            case 1:
                SetDateTime();
                break;
            case 2:
                StartSignalGenerator();
                break;
            case 3:
                StartdBMeter();
                break;
            case 4:
                OutputHardwareCalibration();
                break;
            case 5:
                SetOutPutFit();
                break;
            case 6:
                InputHardwareCalibration();
                break;
            case 7:
                dBMeterScan();
                break;

            };
        }
    } while (1);
}

void loop() {
}
