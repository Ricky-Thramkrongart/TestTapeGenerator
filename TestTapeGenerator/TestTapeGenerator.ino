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
using namespace std;

double randomDouble(double minf, double maxf)
{
    return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}

void splashscreen()
{
    LCD_Helper lcdhelper;
    char stringbuffer[255];
    sprintf(stringbuffer, "  %s", TESTTAPEGENERATOR_SW_VERSION);
    lcdhelper.line[0] = "Test Tape Generator";
    lcdhelper.line[1] = stringbuffer;
    lcdhelper.line[2] = TAPELIST_VERSION;
    lcdhelper.Show();
    delay(2000);
}

#include <Wire.h>
void selftest()
{
    LCD_Helper lcdhelper;
    lcdhelper.line[0] = "Self Test";
    lcdhelper.Show();
    byte devices[] = {0x25, 0x2C, 0x50, 0x68};
    for (int i = 0; i != sizeof(devices) / sizeof(byte); i++)
    {
        Wire.begin();
        Wire.beginTransmission(devices[i]);
        if (Wire.endTransmission() != 0) {
            char stringbuffer[255];
            sprintf(stringbuffer, "Hardware Not Found at %02X", devices[i]);
            lcdhelper.line[0] = stringbuffer;
            lcdhelper.Show();
            exit(EXIT_FAILURE);
        }
    }
    LCD_Helper().Test();

    lcdhelper.line[1] = "DISP:OK EEPROM:OK RTC:OK DATT:OK";
    lcdhelper.Show();
    delay(750);

    lcdhelper.line[2] = "Frequncy response test:";
    lcdhelper.line[3] = "20Hz to 25Khz +/- 0.1 dB :OK";
    lcdhelper.Show();
    delay(2000);
}

class AdjustingReferenceLevelOkDialog : public DialogOk
{
    public:
        std::shared_ptr<TapeInfo> tapeInfo;
        AdjustingReferenceLevelOkDialog(TapeInfo::Tapes Tape): tapeInfo(TapeInfo::Get(Tape))
        {
        }
        void FullUpdate() {
            lcdhelper.line[0] = "Reference Level";
            lcdhelper.line[1] = tapeInfo->ToString()[0];
            lcdhelper.line[2] = "Start recording";
        }
        void Update() {
        }

};

class AdjustingReferenceLevelMonitor : public DialogOk
{
    protected:
    dBMeter dbMeter;
    public:
        std::shared_ptr<TapeInfo> tapeInfo;
        AdjustingReferenceLevelMonitor(TapeInfo::Tapes Tape): tapeInfo(TapeInfo::Get(Tape))
        {
        }
        void Update()
        {
            double Target = tapeInfo->Target;
            uint16_t LeftLevel;
            uint16_t RightLevel;   
            dbMeter.getdB(Target, RightLevel, LeftLevel);
            std::string statuscontrol = StatusControl(0.5, LeftLevel-Target, RightLevel-Target);
            char stringbuffer[255];
            char str_target[6];
            dtostrf(Target, 4, 1, str_target);
            sprintf(stringbuffer, "Target: %s dB  Actuel (L:R): ", str_target);
            digitalWrite(8, LOW);
            lcdhelper.lcd.setCursor(strlen(stringbuffer), 0);
            lcdhelper.lcd.print(statuscontrol.c_str());
        }

        void FullUpdate() {
            double Target = tapeInfo->Target;
            char stringbuffer[255];
            char str_target[6];
            dtostrf(Target, 4, 1, str_target);
            sprintf(stringbuffer, "Target: % s dB  Actuel (L:R):        ", str_target);
            lcdhelper.line[0] = "Reference Level";
            lcdhelper.line[1] = tapeInfo->ToString()[0];
            lcdhelper.line[2] = stringbuffer;
        }
};

class AdjustingReferenceLevelProgress : public Dialog
{
    public:
        std::shared_ptr<TapeInfo> tapeInfo;
        uint16_t i;
        AdjustingReferenceLevelProgress(TapeInfo::Tapes Tape): Dialog(1000),  tapeInfo(TapeInfo::Get(Tape)), i(0)
        {
        }
        void FullUpdate() {
            char stringbuffer[255];
            sprintf(stringbuffer, "Adjustment: %3i %% Complete", i);
            lcdhelper.line[0] = "Reference Level";
            lcdhelper.line[1] = tapeInfo->ToString()[0];
            lcdhelper.line[2] = stringbuffer;
            i += 10;
            if (i > 100) {
                finished = true;
            }
        }
};


class AdjustingRecordLevelProgress : public Dialog
{
    protected:
        SignalGenerator signalGenerator;
    public:
        std::shared_ptr<TapeInfo> tapeInfo;
        std::vector<RecordStep*>::iterator ptr;
        AdjustingRecordLevelProgress(TapeInfo::Tapes Tape): Dialog(1000),  tapeInfo(TapeInfo::Get(Tape)), ptr(tapeInfo->RecordSteps.begin())
        {
        }
        void FullUpdate() {
            char stringbuffer[255];
            sprintf(stringbuffer, "%s (%i/%i)", (*ptr)->ToString().c_str(), (ptr - tapeInfo->RecordSteps.begin()) + 1, (int)tapeInfo->RecordSteps.size());
            lcdhelper.line[0] = "Record Level";
            lcdhelper.line[1] = tapeInfo->ToString()[0];
            lcdhelper.line[2] = stringbuffer;
            signalGenerator.setFreq((*ptr)->Frequency, (*ptr)->Level);
            ptr++;
            if (ptr == tapeInfo->RecordSteps.end()) {
                finished = true;
            }
        }
};

class RecordTestTape : public Dialog
{
    protected:
        SignalGenerator signalGenerator;
    public:
        std::shared_ptr<TapeInfo> tapeInfo;
        std::vector<RecordStep*>::iterator ptr;
        RecordTestTape(TapeInfo::Tapes Tape): Dialog(1000),  tapeInfo(TapeInfo::Get(Tape)), ptr(tapeInfo->RecordSteps.begin())
        {
        }
        void FullUpdate() {
            char stringbuffer[255];
            sprintf(stringbuffer, "%s (%i/%i)", (*ptr)->ToString().c_str(), (ptr - tapeInfo->RecordSteps.begin()) + 1, (int)tapeInfo->RecordSteps.size());
            std::vector<std::string> VUMeter(GetVUMeterStrings(randomDouble(-3, 3), randomDouble(-3, 3)));
            lcdhelper.line[0] = stringbuffer;
            lcdhelper.line[1] = VUMeter[0].c_str();
            lcdhelper.line[2] = VUMeter[1].c_str();
            lcdhelper.line[3] = VUMeter[2].c_str();
            signalGenerator.setFreq((*ptr)->Frequency, (*ptr)->Level);
            ptr++;
            if (ptr == tapeInfo->RecordSteps.end()) {
                finished = true;
            }
        }
};

class MainMenu : public Menu
{
    public:
        MainMenu (): Menu(3) {}
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
            }
            char buffer[255];
            sprintf(buffer, "Current: %i", Current);
            lcdhelper.line[0] = "== Main Menu ==================";
            lcdhelper.line[1] = str;
        }
};

class SelectTape : public Menu
{
    public:
        SelectTape (): Menu(TapeInfo::Tapes::LAST_TAPE) {}
        void FullUpdate() {
            std::shared_ptr<TapeInfo> tapeInfo(TapeInfo::Get(Current));
            std::vector<std::string> strs = tapeInfo->ToString();
            char stringbuffer[255];
            sprintf(stringbuffer, "Tape (%i/%i)", Current + 1, TapeInfo::LAST_TAPE - TapeInfo::FIRST_TAPE);
            lcdhelper.line[0] = strs[0];
            lcdhelper.line[1] = strs[1];
            lcdhelper.line[3] = stringbuffer;
        }
};

void NewTestTape()
{
    TapeInfo::Tapes tape;
    {
        SelectTape selectTape;
        if (! selectTape.Execute()) {
            return;
        }
        tape = selectTape.Current;
    }
    if (! AdjustingReferenceLevelOkDialog(tape).Execute()) {
        return;
    }
    if (! AdjustingReferenceLevelMonitor(tape).Execute()) {
        return;
    }
    if (! AdjustingReferenceLevelProgress(tape).Execute()) {
        return;
    }
    if (! AdjustingRecordLevelProgress(tape).Execute()) {
        return;
    }
    if (! RecordTestTape(tape).Execute()) {
        return;
    }
    if (! PrintProgress(tape).Execute()) {
        return;
    }
}

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
                delay(2000);
                return;
            }
            while (Serial.available() > 0) Serial.read();
        }
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
            };
        }
    } while (1);

}

void loop() {
}
