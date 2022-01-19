// *********************************************************************
//  Test software for Analyzer
// *********************************************************************

#include <ArduinoSTL.h>
#include <bitset>
#include <ArxSmartPtr.h>

#include "TestTapeGenerator.h"
#include "TapeInfo.h"
#include "LCDHelper.h"
#include "Controls.h"
#include "Printer.h"
#include "SignalGenerator.h"
#include "dbMeter.h"
#include <DS3232RTC.h>      // https://github.com/JChristensen/DS3232RTC
#include <Regexp.h>         // https://github.com/nickgammon/Regexp/
#include "Splash.h"
#include "SelfTest.h"
#include "NewTestTape.h"


using namespace std;


class PotentioMeterOutputSelection : public Menu
{
    protected:
        SignalGenerator signalGenerator;
    public:
        PotentioMeterOutputSelection (): Menu(256) {}
        void FullUpdate() {
            char buffer[255];
            sprintf(buffer, "Potention Meter Output: %i", Current);
            signalGenerator.ManualOutPut(Current);
            lcdhelper.line[0] = buffer;
        }
};

class MainMenu : public Menu
{
    public:
        MainMenu (): Menu(4) {}
        void FullUpdate() {
            std::string str;
            switch (Current) {
                case 0:
                    str = "Create Test Tape";
                    break;
                case 1:
                    str = "Set Time";
                    break;
                case 2:
                    str = "Start Signal Generator";
                    break;
                case 3:
                    str = "Output Hardware Calibration";
                    break;
            }
            char buffer[255];
            sprintf(buffer, "Current: %i", Current);
            lcdhelper.line[0] = "== Main Menu ==================";
            lcdhelper.line[1] = str;
        }
};

void SetDateTime()
{
    LCD_Helper lcdhelper;
    RTC_Helper rtchelper;
    lcdhelper.line[0] = "Reading Date Time from Serial Port";
    lcdhelper.line[1] = "Format: [yyyy/mm/dd HH.MM.SS] 115200 Baud";
    lcdhelper.Show();
    Serial.setTimeout(500);
    while (Serial.available() > 0) Serial.read();
    Serial.flush();
    Serial.println(lcdhelper.line[0].c_str());
    Serial.println(lcdhelper.line[1].c_str());

    do {
        if (Serial.available() >= 19) {
            MatchState ms;
            String str(Serial.readString());
            ms.Target(str.c_str());
            char result = ms.Match ("(%d%d%d%d)/(%d%d)/(%d%d) (%d%d)\.(%d%d)\.(%d%d)");
            char cap[256];
            if (result == REGEXP_MATCHED)
            {
                time_t t;
                tmElements_t tm;
                int index = 0;
                ms.GetCapture (cap, index++);
                uint16_t y = atoi(cap);
                tm.Year = CalendarYrToTm(atoi(cap));
                ms.GetCapture (cap, index++);
                tm.Month = atoi(cap);
                ms.GetCapture (cap, index++);
                tm.Day = atoi(cap);
                ms.GetCapture (cap, index++);
                tm.Hour = atoi(cap);
                ms.GetCapture (cap, index++);
                tm.Minute = atoi(cap);
                ms.GetCapture (cap, index++);
                tm.Second = atoi(cap);
                t = makeTime(tm);
                RTC.set(t);        // use the time_t value to ensure correct weekday is set
                setSyncProvider(RTC.get);   // the function to get the time from the RTC
                lcdhelper.line[2] = "Recieved Date Time.";
                lcdhelper.Show();
                Serial.println(lcdhelper.line[2].c_str());
                delay(2000);
                return;
            }
            while (Serial.available() > 0) Serial.read();
        }
    } while (true);
}

void OutputHardwareCalibration (void)
{
    PotentioMeterOutputSelection().Execute();
}

void StartSignalGenerator()
{
    LCD_Helper lcdhelper;
    lcdhelper.line[0] = "Reading frequency and dB from Serial Port";
    lcdhelper.line[1] = "Format: [float float] 115200 Baud";
    lcdhelper.Show();
    Serial.setTimeout(500);
    while (Serial.available() > 0) Serial.read();
    Serial.flush();
    Serial.println(lcdhelper.line[0].c_str());
    Serial.println(lcdhelper.line[1].c_str());
    SignalGenerator signalGenerator;

    do {
        if (Serial.available()) {
            MatchState ms;
            String str(Serial.readString());
            ms.Target(str.c_str());
            char result = ms.Match ("([-+]?[0-9]*\.?[0-9]+) ([-+]?[0-9]*\.?[0-9]+)");
            char cap[256];
            if (result == REGEXP_MATCHED)
            {
                int index = 0;
                ms.GetCapture (cap, index++);
                double freq = atof(cap);
                ms.GetCapture (cap, index++);
                double dB = atof(cap);
                signalGenerator.setFreq(freq, dB);
                char stringbuffer[256];
                char sz_freq[8];
                dtostrf(freq, 4, 1, sz_freq);
                char sz_db[8];
                dtostrf(dB, 4, 1, sz_db);
                sprintf(stringbuffer, "Frequency: %s dB: %s", sz_freq, sz_db);
                lcdhelper.line[2] = "Recieved Frequency and dB.";
                lcdhelper.line[3] = stringbuffer;
                lcdhelper.Show();
                Serial.println(lcdhelper.line[2].c_str());
                Serial.println(lcdhelper.line[3].c_str());
            }
            while (Serial.available() > 0) Serial.read();
        }
        delay(1000);
    } while (true);

    lcdhelper.line[0] = "Setting Signal Generator (1000, 5.0)";
    lcdhelper.Show();
    delay(60000);
}

void setup()
{
    Serial.begin(115200);

    splashscreen();
    selftest();

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
                    OutputHardwareCalibration();
                    break;
            };
        }
    } while (1);

}

void loop() {
}
