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
	  sprintf(stringbuffer,"Hardware Not Found at %02X", devices[i]);
      lcdhelper.line[0] = stringbuffer;
      lcdhelper.Show();
      exit(EXIT_FAILURE);
    }
  }
  lcdhelper.line[1] = "DISP:OK EEPROM:OK RTC:OK DATT:OK";
  lcdhelper.Show();
  delay(750);

  lcdhelper.line[2] = "Frequncy response test:";
  lcdhelper.line[3] = "20Hz to 25Khz +/- 0.1 dB :OK";
  lcdhelper.Show();
  delay(1000);
}

class AdjustingReferenceLevelOkDialog : public Dialog
{
  public:
    std::shared_ptr<TapeInfo> tapeInfo;
    AdjustingReferenceLevelOkDialog(TapeInfo::Tapes Tape): Dialog(0), tapeInfo(TapeInfo::Get(Tape))
    {
    }
    void UpdateLCD() {
      lcdhelper.line[0] = "Reference Level";
      lcdhelper.line[1] = tapeInfo->ToString()[0];
      lcdhelper.line[2] = "Start recording";
    }
};

class AdjustingReferenceLevelMonitor : public Dialog
{
  public:
    std::shared_ptr<TapeInfo> tapeInfo;
    AdjustingReferenceLevelMonitor(TapeInfo::Tapes Tape): Dialog(1000),  tapeInfo(TapeInfo::Get(Tape))
    {
    }
    void UpdateLCD() {
      double Target = tapeInfo->Target;
      double LeftLevel = Target + randomDouble(-1.0, 1.0);
      double RightLevel = Target + randomDouble(-1.0, 1.0);
      char LeftLevelStatus = '=';
      if (LeftLevel < 0.5)
      {
        LeftLevelStatus = '<';
      }
      if (LeftLevel > 0.5)
      {
        LeftLevelStatus = '>';
      }

      char RightLevelStatus = '=';
      if (RightLevel < 0.5)
      {
        RightLevelStatus = '<';
      }
      if (RightLevel > 0.5)
      {
        RightLevelStatus = '>';
      }

      char stringbuffer[255];
      char str_target[6];
      dtostrf(Target, 4, 1, str_target);
      sprintf(stringbuffer, "Target: %s dB  Actuel (L:R): %c:%c", str_target, LeftLevelStatus, RightLevelStatus);
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
    void UpdateLCD() {
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
  public:
    std::shared_ptr<TapeInfo> tapeInfo;
    std::vector<RecordStep*>::iterator ptr;
    AdjustingRecordLevelProgress(TapeInfo::Tapes Tape): Dialog(1000),  tapeInfo(TapeInfo::Get(Tape)), ptr(tapeInfo->RecordSteps.begin())
    {
    }
    void UpdateLCD() {
      char stringbuffer[255];
      sprintf(stringbuffer, "%s (%i/%i)", (*ptr)->ToString().c_str(), (ptr - tapeInfo->RecordSteps.begin()) + 1, (int)tapeInfo->RecordSteps.size());
      lcdhelper.line[0] = "Record Level";
      lcdhelper.line[1] = tapeInfo->ToString()[0];
      lcdhelper.line[2] = stringbuffer;
      ptr++;
      if (ptr == tapeInfo->RecordSteps.end()) {
        finished = true;
      }
    }
};

class RecordTestTape : public Dialog
{
  public:
    std::shared_ptr<TapeInfo> tapeInfo;
    std::vector<RecordStep*>::iterator ptr;
    RecordTestTape(TapeInfo::Tapes Tape): Dialog(1000),  tapeInfo(TapeInfo::Get(Tape)), ptr(tapeInfo->RecordSteps.begin())
    {
    }
    void UpdateLCD() {
      char stringbuffer[255];
      sprintf(stringbuffer, "%s (%i/%i)", (*ptr)->ToString().c_str(), (ptr - tapeInfo->RecordSteps.begin()) + 1, (int)tapeInfo->RecordSteps.size());
      std::vector<std::string> VUMeter(GetVUMeterStrings(randomDouble(-3, 3), randomDouble(-3, 3)));
      lcdhelper.line[0] = stringbuffer;
      lcdhelper.line[1] = VUMeter[0].c_str();
      lcdhelper.line[2] = VUMeter[1].c_str();
      lcdhelper.line[3] = VUMeter[2].c_str();
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
    void UpdateLCD() {
      std::string str;
      switch (Current) {
        case 0:
          str = "Create Test Tape";
          break;
        case 1:
          str = "Set Time";
          break;
        case 2:
          str = "Mode";
          break;
      }
      char buffer[255];
      sprintf(buffer, "Current: %i", Current);
      lcdhelper.line[0] = "== Main Menu ===============";
      lcdhelper.line[1] = str;
    }
};

class SelectTape : public Menu
{
  public:
    SelectTape (): Menu(TapeInfo::Tapes::LAST_TAPE) {}
    void UpdateLCD() {
      std::shared_ptr<TapeInfo> tapeInfo(TapeInfo::Get(Current));
      std::vector<std::string> strs = tapeInfo->ToString();
      char stringbuffer[255];
      sprintf(stringbuffer, "Tape (%i/%i)", Current+1, TapeInfo::LAST_TAPE-TapeInfo::FIRST_TAPE);
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

void setup() {


	
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
        case 2:
          break;
      };
    }
  } while (1);
}

void loop() {
};
