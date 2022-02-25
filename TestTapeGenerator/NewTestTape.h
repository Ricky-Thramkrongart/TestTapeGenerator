#pragma once

#include "FindDb.h"
#include "Beep.h"

//double randomDouble(double minf, double maxf)
//{
//    return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
//}

class StartRecording : public DialogOk
{
public:
    TapeInfo* tapeInfo;
    StartRecording(TapeInfo* tapeInfo_) : tapeInfo(tapeInfo_)
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

class AmplificationAdjustment: public DialogOk
{
protected:
    int manual_calibration_ok_count;
public:
    TapeInfo* tapeInfo;
    const uint32_t Targetfreq;

    AmplificationAdjustment(TapeInfo* tapeInfo_) : tapeInfo(tapeInfo_), Targetfreq(1000)
    {
        //Serial.print(F("DBOUT_MAX_SERVICE: ")); Serial.print(DBOUT_MAX_SERVICE); Serial.print(F(" Amp. Adj.: ")); Serial.println(tapeInfo->GetAmplificationAdjustment());
        double d = -tapeInfo->GetAmplificationAdjustment();
        System::OutPutOn();
        SignalGenerator::Get().setFreq(Targetfreq, { d, d });
    }
    ~AmplificationAdjustment()
    {
        System::PopRelayStack();
    }
    void Update()
    {
        double Target = 0;
        dBMeter::Measurement m({ Target ,Target });
        dBMeter::Get().GetdB(m);
        Serial.println(m.String(2));
        Serial.println(m.ToString().c_str());

         constexpr double std_dev = 1.0;
        std::string statuscontrol = StatusControl(std_dev, m.dBIn.first - Target, m.dBIn.second - Target);
        if (fabs(m.dBIn.first - Target) < std_dev && fabs(m.dBIn.second - Target) < std_dev) {
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
            buttonPanel.returncode = ButtonPanel<DialogOk>::IDOK;
        }
    }

    void FullUpdate() {
        double Target = 0;
        cSF(sf_line, 41);
        sf_line.print(F("Target: "));
        sf_line.print(Target, 1, 4);
        sf_line.print(F(" dBm  Actuel (L:R):        "));
        lcdhelper.Line(0, F("Amplification Adjustment"));
        lcdhelper.Line(1, tapeInfo->ToString()[0].c_str());
        lcdhelper.Line(2, sf_line);
    }
};

class RecordLevelAdjustment : public Dialog
{
public:
    TapeInfo* tapeInfo;
    std::vector<RecordStep*>::iterator ptr;
    RecordLevelAdjustment(TapeInfo* tapeInfo_) : Dialog(1000), tapeInfo(tapeInfo_), ptr(tapeInfo->RecordSteps.begin())
    {
        System::OutPutOn();
    }
    ~RecordLevelAdjustment()
    {
        System::PopRelayStack();
    }
    void FullUpdate() {
        lcdhelper.Line(0, F("Adjusting Record Level"));
        lcdhelper.Line(1, tapeInfo->ToString()[0].c_str());
        lcdhelper.Line(3, "");
        lcdhelper.Show();

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
        std::pair<double, double> start_guess{ x0.first - tapeInfo->GetAmplificationAdjustment(),  x0.second - tapeInfo->GetAmplificationAdjustment() };
        (*ptr)->RecordLevel = FinddB((*ptr)->Frequency, x0, start_guess, (*ptr)->e, lcdhelper);
        SignalGenerator::Get().setFreq((*ptr)->Frequency, (*ptr)->RecordLevel);
        delay(1000); //Setteling time
        dBMeter::Measurement m((*ptr)->RecordLevel);
        dBMeter::Get().GetdB(m);
        lcdhelper.Line(2, SignalGenerator::String((*ptr)->Frequency, (*ptr)->RecordLevel));
        lcdhelper.Line(3, m.String());
        Serial.println(m.String());
        lcdhelper.Show(2000);
        Beep();
        ptr++;
        if (ptr == tapeInfo->RecordSteps.end()) {
            finished = true;
        }
    }
};

class RewindTape: public DialogOk
{
public:
    TapeInfo* tapeInfo;
    RewindTape(TapeInfo* tapeInfo_) : tapeInfo(tapeInfo_)
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
    unsigned long ms_progress;
public:
    TapeInfo* tapeInfo;
    std::vector<RecordStep*>::iterator ptr;
    RecordTestTape(TapeInfo* tapeInfo_) : Dialog(1000), tapeInfo(tapeInfo_), ptr(tapeInfo->RecordSteps.begin()), ms_progress(millis())
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
        cSF(sf_line2, 41);
        sf_line.print((*ptr)->ToString().c_str());
        sf_line.print(F(" ("));
        sf_line.print((ptr - tapeInfo->RecordSteps.begin()) + 1);
        sf_line.print(F("/"));
        sf_line.print((int)tapeInfo->RecordSteps.size());
        sf_line.print(F(")"));
        lcdhelper.Line(2, sf_line);
        sf_line2.print(F("Blank Space: "));  sf_line2.print(tapeInfo->Pause); sf_line2.print(F("s"));
        lcdhelper.Line(3, sf_line2);
        lcdhelper.Show();
        lcdhelper.Show(Serial);
        std::pair<double, double> x0((*ptr)->RecordLevel);
        uint32_t f((*ptr)->Frequency);
        SignalGenerator::Get().setFreq(f, x0);
        dBMeter::Measurement m(x0);
        delay(tapeInfo->Pause*1000); //Setteling time //Blank space
        System::OutPutOn();
        unsigned long ms = millis();
        std::pair<double, double> stdsum{ 0.0, 0.0 };
        std::pair<double, double> std;
        uint16_t count = 0;

        lcdhelper.Line(1, F(""));
        lcdhelper.Line(2, F(""));
        lcdhelper.Line(3, F(""));

        lcdhelper.Line(0, sf_line);
        do {
            sf_line2 = sf_line; sf_line2.print(F(" ")); sf_line2.print((*ptr)->Time - (millis() - ms) / 1000.0,0,3); sf_line2.print(F("s ")); sf_line2.print((millis() - ms_progress) / (tapeInfo->Length * 10.0),0,3); sf_line2.print(F("%"));
            lcdhelper.Line(0, sf_line2);
            lcdhelper.Show();

            dBMeter::Get().GetdB(m);
            count++;

            stdsum.first += square(m.dBIn.first - (*ptr)->Level);
            stdsum.second += square(m.dBIn.second - (*ptr)->Level);
            std.first = sqrt(stdsum.first / count);
            std.second = sqrt(stdsum.second / count);
            std::vector<std::string> VUMeter(GetVUMeterStrings(std.first, std.second));
            lcdhelper.Line(1, VUMeter[0].c_str());
            lcdhelper.Line(2, VUMeter[1].c_str());
            lcdhelper.Line(3, VUMeter[2].c_str());
            lcdhelper.Show();
            lcdhelper.Show(Serial);
         } while (millis() < (*ptr)->Time*1000.0 + ms);
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
    if (StartRecording(tapeInfo.get()).Execute()!= ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    if (AmplificationAdjustment(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    if (RecordLevelAdjustment(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    if (RewindTape(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    if (RecordTestTape(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
        return;
    }
//    if (!PrintProgress(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
//           return;
//    }
}
