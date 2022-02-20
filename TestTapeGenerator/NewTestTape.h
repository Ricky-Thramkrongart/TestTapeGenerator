#pragma once

#include "FindDb.h"
#include "Beep.h"

//double randomDouble(double minf, double maxf)
//{
//    return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
//}

class AdjustingReferenceLevelOkDialog : public DialogOk
{
public:
    TapeInfo* tapeInfo;
    AdjustingReferenceLevelOkDialog(TapeInfo* tapeInfo_) : tapeInfo(tapeInfo_)
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

class ManualReferenceLevelAdjustment : public DialogOk
{
protected:
    SignalGenerator signalGenerator;
    dBMeter dbMeter;
    int manual_calibration_ok_count;
public:
    TapeInfo* tapeInfo;
    const uint32_t Targetfreq;

    ManualReferenceLevelAdjustment(TapeInfo* tapeInfo_) : tapeInfo(tapeInfo_), Targetfreq(1000)
    {
        double d = tapeInfo->Target;
        System::OutPutOn();
        signalGenerator.setFreq(Targetfreq, { d, d });
    }
    ~ManualReferenceLevelAdjustment()
    {
        System::PopRelayStack();
    }
    void Update()
    {
        double Target = tapeInfo->Target;
        dBMeter::Measurement m({ Target ,Target });
        dbMeter.GetdB(m);
        Serial.println(m.String(2));

        std::string statuscontrol = StatusControl(.5, m.dBIn.first - Target, m.dBIn.second - Target);
        if (fabs(m.dBIn.first - Target) < .5 && fabs(m.dBIn.second - Target) < .5) {
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
        lcdhelper.lcd.setCursor(0, 1);
        lcdhelper.lcd.print(m.String(2));

        if (manual_calibration_ok_count >= 5) {
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
        lcdhelper.Line(0, F("Manual Reference Level Adjustment"));
        lcdhelper.Line(1, tapeInfo->ToString()[0].c_str());
        lcdhelper.Line(2, sf_line);
    }
};

class AdjustingRecordLevel : public Dialog
{
protected:
    SignalGenerator signalGenerator;
    dBMeter dbMeter;
public:
    TapeInfo* tapeInfo;
    std::vector<RecordStep*>::iterator ptr;
    AdjustingRecordLevel(TapeInfo* tapeInfo_) : Dialog(1000), tapeInfo(tapeInfo_), ptr(tapeInfo->RecordSteps.begin())
    {
        System::OutPutOn();
    }
    ~AdjustingRecordLevel()
    {
        System::PopRelayStack();
    }
    void FullUpdate() {
        lcdhelper.Line(0, F("Adjusting Record Level"));
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
        System::PrintRelayState();
        std::pair<double, double> x0({ (*ptr)->Level, (*ptr)->Level });
        (*ptr)->RecordLevel = FindDb(signalGenerator, dbMeter, (*ptr)->Frequency, x0, x0, (*ptr)->e, lcdhelper);
        signalGenerator.setFreq((*ptr)->Frequency, (*ptr)->RecordLevel);
        delay(1000); //Setteling time
        dBMeter::Measurement m((*ptr)->RecordLevel);
        dbMeter.GetdB(m);
        lcdhelper.Line(2, SignalGenerator::String((*ptr)->Frequency, (*ptr)->RecordLevel));
        lcdhelper.Line(3, m.String());
        Serial.println(m.String());
        lcdhelper.Show();
        Beep();
        ptr++;
        if (ptr == tapeInfo->RecordSteps.end()) {
            finished = true;
        }
    }
};

class RecordTestTapeOkDialog: public DialogOk
{
public:
    TapeInfo* tapeInfo;
    RecordTestTapeOkDialog(TapeInfo* tapeInfo_) : tapeInfo(tapeInfo_)
    {
    }
    void FullUpdate() {
        lcdhelper.Line(0, F("Record Test Tape"));
        lcdhelper.Line(1, tapeInfo->ToString()[0].c_str());
        lcdhelper.Line(2, F("Rewind Tape and Start Recording"));
    }
    void Update() {
    }

};

class RecordTestTape : public Dialog
{
protected:
    SignalGenerator signalGenerator;
    dBMeter dbMeter;
public:
    TapeInfo* tapeInfo;
    std::vector<RecordStep*>::iterator ptr;
    RecordTestTape(TapeInfo* tapeInfo_) : Dialog(1000), tapeInfo(tapeInfo_), ptr(tapeInfo->RecordSteps.begin())
    {
        System::OutPutOff();
    }
    ~RecordTestTape()
    {
        System::PopRelayStack();
    }
    void FullUpdate() {
        lcdhelper.Line(0, F("Recording Test Tape"));
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
        std::pair<double, double> x0((*ptr)->RecordLevel);
        uint32_t f((*ptr)->Frequency);
        signalGenerator.setFreq(f, x0);
        dBMeter::Measurement m(x0);
        delay(2000); //Setteling time //Blank space
        System::OutPutOn();
        long ms = millis();
        std::pair<double, double> stdsum{ 0.0, 0.0 };
        std::pair<double, double> std;
        uint16_t count = 0;

        lcdhelper.Line(0, sf_line);
        do {
            dbMeter.GetdB(m);
            count++;

            stdsum.first += square(m.dBIn.first - (*ptr)->Level);
            stdsum.second += square(m.dBIn.second - (*ptr)->Level);
            std.first = sqrt(stdsum.first / count);
            std.second = sqrt(stdsum.second / count);
            cSF(sf_line, 41);

            std::vector<std::string> VUMeter(GetVUMeterStrings(std.first, std.second));
            lcdhelper.Line(1, VUMeter[0].c_str());
            lcdhelper.Line(2, VUMeter[1].c_str());
            lcdhelper.Line(3, VUMeter[2].c_str());
            lcdhelper.Show();
            lcdhelper.Show(Serial);
         } while (millis() - ms < (*ptr)->Time*1000);
            //} while (millis() - ms < 20000);
        System::PopRelayStack();

        Beep();
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
    //std::shared_ptr<TapeInfo> tapeInfo(TapeInfo::Get(TapeInfo::AKAI_GX_75_95_TEST_TAPE));
    std::shared_ptr<TapeInfo> tapeInfo;
    {
        SelectTape selectTape;
        if (selectTape.Execute() != ButtonPanel<BasePanel>::IDOK) {
            return;
        }
        tapeInfo = std::shared_ptr<TapeInfo>(TapeInfo::Get((TapeInfo::Tapes)selectTape.Current));
    }
    if (AdjustingReferenceLevelOkDialog(tapeInfo.get()).Execute()!= ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    if (ManualReferenceLevelAdjustment(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    if (AdjustingRecordLevel(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    if (RecordTestTapeOkDialog(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    if (RecordTestTape(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    //    if (!PrintProgress(tape).Execute()) {
    //        return;
    //    }
}
