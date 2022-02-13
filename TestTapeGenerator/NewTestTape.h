#pragma once

#include "FindDb.h"
#include "Beep.h"

double randomDouble(double minf, double maxf)
{
    return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}

class AdjustingReferenceLevelOkDialog : public DialogOk
{
public:
    std::shared_ptr<TapeInfo> tapeInfo;
    AdjustingReferenceLevelOkDialog(TapeInfo::Tapes Tape) : tapeInfo(TapeInfo::Get(Tape))
    {
    }
    void FullUpdate() {
        lcdhelper.Line(0, F("Reference Level"));
        lcdhelper.Line(1, tapeInfo->ToString()[0].c_str());
        lcdhelper.Line(2, F("Start recording"));
    }
    void Update() {
    }

};

class AdjustingReferenceLevelMonitor : public DialogOk
{
protected:
    SignalGenerator signalGenerator;
    dBMeter dbMeter;
    int manual_calibration_ok_count;
public:
    std::shared_ptr<TapeInfo> tapeInfo;
    const uint32_t Targetfreq;

    AdjustingReferenceLevelMonitor(TapeInfo::Tapes Tape) : tapeInfo(TapeInfo::Get(Tape)), Targetfreq(1000)
    {
        double d = tapeInfo->Target - 6;
        signalGenerator.setFreq(Targetfreq, { d, d });
        System::UnMute();
    }
    ~AdjustingReferenceLevelMonitor()
    {
        System::PopRelayStack();
    }
    void Update()
    {
        double Target = tapeInfo->Target;
        dBMeter::Measurement m({ Target ,Target });
        dbMeter.GetdB(m);
        Serial.println(m.String(2));
        Serial.println(m.ToString().c_str());

        std::string statuscontrol = StatusControl(1.5, m.dBIn.first - Target, m.dBIn.second - Target);
        if (fabs(m.dBIn.first - Target) < 1.5 && fabs(m.dBIn.second - Target) < 1.5) {
            manual_calibration_ok_count++;
            Beep();
        }
        else {
            manual_calibration_ok_count = 0;
        }

        cSF(sf_line, 41);
        sf_line.print(F("Target: "));
        sf_line.print(Target, 1, 4);
        sf_line.print(F(" dBm  Actuel (L:R): "));
        digitalWrite(8, LOW);
        lcdhelper.lcd.setCursor(sf_line.length(), 0);
        lcdhelper.lcd.print(statuscontrol.c_str());
        lcdhelper.lcd.setCursor(0,1);
        lcdhelper.lcd.print(m.String(2));

        if (manual_calibration_ok_count >= 3) {
            Beep();
            dbMeter.Cabling(signalGenerator);
            buttonPanel.returncode = ButtonPanel<DialogOk>::IDOK;
        }
    }

    void FullUpdate() {
        double Target = tapeInfo->Target;
        cSF(sf_line, 41);
        sf_line.print(F("Target: "));
        sf_line.print(Target, 1, 4);
        sf_line.print(F(" dBm  Actuel (L:R):        "));
        lcdhelper.Line(0, F("Reference Level (1/2)"));
        lcdhelper.Line(1, tapeInfo->ToString()[0].c_str());
        lcdhelper.Line(2, sf_line);
    }
};

class AdjustingRecordLevelProgress : public Dialog
{
protected:
    SignalGenerator signalGenerator;
    dBMeter dbMeter;
public:
    std::shared_ptr<TapeInfo> tapeInfo;
    std::vector<RecordStep*>::iterator ptr;
    AdjustingRecordLevelProgress(TapeInfo::Tapes Tape) : Dialog(1000), tapeInfo(TapeInfo::Get(Tape)), ptr(tapeInfo->RecordSteps.begin())
    {
        System::UnMute();
    }
    ~AdjustingRecordLevelProgress()
    {
        System::PopRelayStack();
    }
    void FullUpdate() {
        lcdhelper.Line(0, F("Record Level"));
        lcdhelper.Line(1, tapeInfo->ToString()[0].c_str());

        cSF(sf_line, 41);
        sf_line.print((*ptr)->ToString().c_str());
        sf_line.print(F(" ("));
        sf_line.print((ptr - tapeInfo->RecordSteps.begin()) + 1);
        sf_line.print(F("/"));
        sf_line.print((int)tapeInfo->RecordSteps.size());
        sf_line.print(F(")"));
        lcdhelper.Line(2, sf_line);
        Serial.println(sf_line);
        (*ptr)->RecordLevel = FindDb(signalGenerator, dbMeter, (*ptr)->Frequency, { (*ptr)->Level, (*ptr)->Level });
        lcdhelper.Line(3, dBMeter::Measurement((*ptr)->RecordLevel).String(2));
        Beep();
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
    RecordTestTape(TapeInfo::Tapes Tape) : Dialog(1000), tapeInfo(TapeInfo::Get(Tape)), ptr(tapeInfo->RecordSteps.begin())
    {
    }
    void FullUpdate() {

        cSF(sf_line, 41);
        sf_line.print((*ptr)->ToString().c_str());
        sf_line.print(F(" ("));
        sf_line.print((ptr - tapeInfo->RecordSteps.begin()) + 1);
        sf_line.print(F("/"));
        sf_line.print((int)tapeInfo->RecordSteps.size());
        sf_line.print(F(")"));
        std::vector<std::string> VUMeter(GetVUMeterStrings(randomDouble(-3, 3), randomDouble(-3, 3)));
        lcdhelper.Line(0, sf_line.c_str());
        lcdhelper.Line(1, VUMeter[0].c_str());
        lcdhelper.Line(2, VUMeter[1].c_str());
        lcdhelper.Line(3, VUMeter[2].c_str());
        signalGenerator.setFreq((*ptr)->Frequency, { (*ptr)->Level, (*ptr)->Level });
        ptr++;
        if (ptr == tapeInfo->RecordSteps.end()) {
            finished = true;
        }
    }
};

class SelectTape : public Menu
{
public:
    SelectTape() : Menu(TapeInfo::Tapes::LAST_TAPE) {}
    void FullUpdate() {
        std::shared_ptr<TapeInfo> tapeInfo(TapeInfo::Get(Current));
        std::vector<std::string> strs = tapeInfo->ToString();
        lcdhelper.Line(0, strs[0].c_str());
        lcdhelper.Line(1, strs[1].c_str());
    }
};

void NewTestTape()
{
    TapeInfo::Tapes tape;
    {
        SelectTape selectTape;
        if (!selectTape.Execute()) {
            return;
        }
        tape = selectTape.Current;
    }
    if (!AdjustingReferenceLevelOkDialog(tape).Execute()) {
        return;
    }
    if (!AdjustingReferenceLevelMonitor(tape).Execute()) {
        return;
    }
    if (!AdjustingRecordLevelProgress(tape).Execute()) {
        return;
    }
 //   if (!RecordTestTape(tape).Execute()) {
 //       return;
 //   }
//    if (!PrintProgress(tape).Execute()) {
//        return;
//    }
}