#pragma once

#include <ArduinoSTL.h>
#include <map>
#include "FindDb.h"
#include "Beep.h"

class StartRecording : public DialogOk
{
public:
    TapeInfo* tapeInfo;
    StartRecording(TapeInfo* tapeInfo_) : tapeInfo(tapeInfo_)
    {
    }
    void FullUpdate() {
        lcdhelper.Line(0, F("Amplification Adjustment"));
        lcdhelper.Line(1, tapeInfo->ToString0());
        lcdhelper.Line(2, F("Start recording"));
    }
    void Update() {
    }

};

class AmplificationAdjustment : public DialogOk
{
protected:
    int manual_calibration_ok_count;
public:
    TapeInfo* tapeInfo;
    const uint32_t Targetfreq;

    AmplificationAdjustment(TapeInfo* tapeInfo_) : tapeInfo(tapeInfo_), Targetfreq(1000)
    {
        Serial.print(F("DBOUT_MAX_SERVICE: ")); Serial.print(DBOUT_MAX_SERVICE); Serial.print(F(" Amp. Adj.: ")); Serial.println(tapeInfo->GetAmplificationAdjustment());
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
        //Serial.println(m.ToString().c_str());

        constexpr double std_dev = 1.0;
        String statuscontrol = StatusControl(std_dev, m.dBIn.first - Target, m.dBIn.second - Target);
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
        lcdhelper.Line(1, tapeInfo->ToString0());
        lcdhelper.Line(2, sf_line);
    }
};

class ValidateTapeRecorder : public DialogOk
{
public:
    TapeInfo* tapeInfo;
    typedef std::map<uint32_t, std::pair<int8_t, int8_t> > ValidationMap;
    ValidationMap   Validation;
    ValidationMap::iterator ptr;
    uint16_t v;

    ValidateTapeRecorder(TapeInfo* tapeInfo_) : tapeInfo(tapeInfo_), v(0)
    {
        for (std::vector<RecordStep*>::iterator i = tapeInfo->RecordSteps.begin(); tapeInfo->RecordSteps.end() != i; ++i) {
            ValidationMap::iterator j = Validation.find((*i)->Frequency);
            if (Validation.end() == j) {
                Validation[(*i)->Frequency] = { (*i)->Level, (*i)->Level };
            }
            else {
                j->second.first = std::min(j->second.first, (*i)->Level);
                j->second.second = std::max(j->second.second, (*i)->Level);
            }
        }
        ptr = Validation.begin();

        System::OutPutOn();
    }
    ~ValidateTapeRecorder()
    {
        System::PopRelayStack();
    }
    void FullUpdate() {
    }
    void Update() {
        cSF(sf_line, 41);
        sf_line.print(F("Validate Tape Recorder "));
        sf_line.print(freeMemory());
        sf_line.print(F("B"));
        lcdhelper.Line(0, sf_line);
        lcdhelper.Line(1, tapeInfo->ToString0());
        lcdhelper.Line(3, "");

        uint32_t frequency(ptr->first);
        int8_t required_min_level(ptr->second.first);
        int8_t required_max_level(ptr->second.second);

        sf_line.clear();
        sf_line.print(frequency);
        sf_line.print(F("Hz ["));
        sf_line.print(required_min_level);
        sf_line.print(F(":"));
        sf_line.print(required_max_level);
        sf_line.print(F("]dBm  ("));
        sf_line.print(++v);
        sf_line.print(F("/"));
        sf_line.print((int)Validation.size());
        sf_line.print(F(")"));
        lcdhelper.Line(2, sf_line);
        Serial.println(sf_line);
        System::PrintRelayState();
        lcdhelper.Show();

        std::pair<double, double> dbout_max{ DBOUT_MAX ,DBOUT_MAX };
        dBMeter::Measurement m_max(dbout_max);

        SignalGenerator::Get().setFreq(frequency, dbout_max);
        delay(2000); //Setteling time
        dBMeter::Get().GetdB(m_max);
        if (m_max.dBIn.first < required_max_level || m_max.dBIn.second < required_max_level) {
            sf_line.clear();
            sf_line.print(F("!! Device Max Level < Level: ")); sf_line.print(required_max_level);
            lcdhelper.Line(2, sf_line);
            lcdhelper.Line(3, m_max.String());
            lcdhelper.Show(Serial);
            lcdhelper.Show(10000);
            buttonPanel.returncode = ButtonPanel<DialogOk>::IDABORT;
        }

        std::pair<double, double> dbout_min{ DBOUT_MIN ,DBOUT_MIN };
        dBMeter::Measurement m_min(dbout_min);
        SignalGenerator::Get().setFreq(frequency, dbout_min);
        delay(2000); //Setteling time
        dBMeter::Get().GetdB(m_min);

        if (m_min.dBIn.first > required_min_level || m_min.dBIn.second > required_min_level) {
            sf_line.clear();
            sf_line.print(F("!! Device Min Level > Level: ")); sf_line.print(required_min_level);
            lcdhelper.Line(2, sf_line);
            lcdhelper.Line(3, m_min.String());
            lcdhelper.Show(Serial);
            lcdhelper.Show(10000);
            buttonPanel.returncode = ButtonPanel<DialogOk>::IDABORT;
        }

        if (buttonPanel.returncode != ButtonPanel<DialogOk>::IDABORT) {
            sf_line.clear();
            sf_line.print(F("Ok: [("));
            sf_line.print(m_min.dBIn.first, 1, 5, true);
            sf_line.print(F(","));
            sf_line.print(m_min.dBIn.second, 1, 5, true);
            sf_line.print(F("):("));
            sf_line.print(m_max.dBIn.first, 1, 5, true);
            sf_line.print(F(","));
            sf_line.print(m_max.dBIn.second, 1, 5, true);
            sf_line.print(F(")]dBm"));
            lcdhelper.Line(3, sf_line);
            lcdhelper.Show(Serial);
            lcdhelper.Show(2000);
        }
        ptr++;
        if (Validation.end() == ptr) {
            buttonPanel.returncode = ButtonPanel<DialogOk>::IDOK;
        }
    }
};

class RecordLevelAdjustment : public Dialog
{
public:
    TapeInfo* tapeInfo;
    std::vector<RecordStep*>::iterator ptr;
    std::vector<RecordStep*> RecordSteps;
    std::vector<std::pair<double, double>>& RecordLevels;

    RecordLevelAdjustment(TapeInfo* tapeInfo_) : Dialog(1000), tapeInfo(tapeInfo_), RecordLevels(tapeInfo->RecordLevels)
    {
        RecordSteps = tapeInfo->RecordSteps;
        std::sort(RecordSteps.begin(), RecordSteps.end());
        auto last = std::unique(RecordSteps.begin(), RecordSteps.end());
        if (last == RecordSteps.end()) {
            RecordSteps = tapeInfo->RecordSteps;
        }
        else {
            RecordSteps.erase(last, RecordSteps.end());
        }
        ptr = RecordSteps.begin();

        System::OutPutOn();
    }
    ~RecordLevelAdjustment()
    {
        System::PopRelayStack();
    }
    void FullUpdate() {
        cSF(sf_line, 41);
        sf_line.print(F("Adjusting Record Level "));
        sf_line.print(freeMemory());
        sf_line.print(F("B"));
        lcdhelper.Line(0, sf_line);
        lcdhelper.Line(1, tapeInfo->ToString0());
        lcdhelper.Line(3, "");

        sf_line.clear();
        sf_line.print((*ptr)->ToString().c_str());
        sf_line.print(F(" ("));
        sf_line.print((ptr - RecordSteps.begin()) + 1);
        sf_line.print(F("/"));
        sf_line.print((int)RecordSteps.size());
        sf_line.print(F(")"));
        lcdhelper.Line(2, sf_line);
        Serial.println(sf_line);
        lcdhelper.Show();
        std::pair<double, double>& RecordLevel = RecordLevels[ptr - RecordSteps.begin()];
        if (RecordLevel.second == std::numeric_limits<double>::min()) {
            System::PrintRelayState();
            std::pair<double, double> x0({ (*ptr)->Level, (*ptr)->Level });
            std::pair<double, double> start_guess{ x0.first - tapeInfo->GetAmplificationAdjustment(),  x0.second - tapeInfo->GetAmplificationAdjustment() };
            RecordLevel = FinddB((*ptr)->Frequency, x0, start_guess, lcdhelper);
            SignalGenerator::Get().setFreq((*ptr)->Frequency, RecordLevel);
            delay(1000); //Setteling time
            dBMeter::Measurement m(RecordLevel);
            dBMeter::Get().GetdB(m);
            lcdhelper.Line(2, SignalGenerator::String((*ptr)->Frequency, RecordLevel));
            lcdhelper.Line(3, m.String());
            Serial.println(m.String());
            lcdhelper.Show(2000);
        }

        ptr++;
        if (ptr == RecordSteps.end()) {
            finished = true;
        }
    }
};

class RewindTape : public DialogOk
{
public:
    TapeInfo* tapeInfo;
    RewindTape(TapeInfo* tapeInfo_) : tapeInfo(tapeInfo_)
    {
    }
    void FullUpdate() {
        lcdhelper.Line(0, F("Record Test Tape"));
        lcdhelper.Line(1, tapeInfo->ToString0());
        lcdhelper.Line(2, F("Rewind Tape and Start Recording"));
    }
    void Update() {
    }

};

class RecordTestTape : public Dialog
{
protected:
    unsigned long ms_progress;
    double std_max;
    std::vector<RecordStep*>::iterator ptr_std_max;
    bool show_status;
public:
    TapeInfo* tapeInfo;
    std::vector<RecordStep*>::iterator ptr;
    RecordTestTape(TapeInfo* tapeInfo_) : Dialog(1000), tapeInfo(tapeInfo_), ptr(tapeInfo->RecordSteps.begin()), ms_progress(millis()), std_max(0.0), ptr_std_max(tapeInfo->RecordSteps.begin()), show_status(true)
    {
        System::OutPutOff();
    }
    ~RecordTestTape()
    {
        System::PopRelayStack();
    }
    void FullUpdate() {
        if (ptr == tapeInfo->RecordSteps.end()) {
            if (show_status) {
                cSF(sf_line, 41);
                sf_line.print(F("Std Max: ")); sf_line.print(std_max);
                lcdhelper.Line(1, (*ptr_std_max)->ToString().c_str());
                lcdhelper.Line(2, sf_line);
                lcdhelper.Line(3, F(""));
                lcdhelper.Show(Serial);
                lcdhelper.Show();
                show_status = false;
            }
            //finished = true;
        }
        else {
            cSF(sf_line, 41);
            sf_line.print(F("Recording Test Tape "));
            sf_line.print(freeMemory());
            sf_line.print(F("B"));
            lcdhelper.Line(0, sf_line);
            lcdhelper.Line(1, tapeInfo->ToString0());

            sf_line.clear();
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
            std::pair<double, double>& RecordLevel = tapeInfo->RecordLevels[ptr - tapeInfo->RecordSteps.begin()];
            std::pair<double, double> x0(RecordLevel);
            uint32_t f((*ptr)->Frequency);
            SignalGenerator::Get().setFreq(f, x0);
            dBMeter::Measurement m(x0);
            delay(tapeInfo->Pause * 1000); //Setteling time //Blank space
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
                sf_line2 = sf_line; sf_line2.print(F(" ")); sf_line2.print((*ptr)->Time - (millis() - ms) / 1000.0, 0, 3); sf_line2.print(F("s ")); sf_line2.print(std::min((millis() - ms_progress) / (tapeInfo->Length * 10.0), 100.0), 0, 3); sf_line2.print(F("%"));
                lcdhelper.Line(0, sf_line2);
                lcdhelper.Show();

                dBMeter::Get().GetdB(m);
                count++;
                double std_max_ = std::max(fabs(m.dBIn.first - (*ptr)->Level), fabs(m.dBIn.second - (*ptr)->Level));
                if (std_max_ > std_max) {
                    std_max = std_max_;
                    ptr_std_max = ptr;
                }
                stdsum.first += square(m.dBIn.first - (*ptr)->Level);
                stdsum.second += square(m.dBIn.second - (*ptr)->Level);
                std.first = sqrt(stdsum.first / count);
                std.second = sqrt(stdsum.second / count);
                GetVUMeterStrings(std.first, std.second, lcdhelper);
                lcdhelper.Show();
                lcdhelper.Show(Serial);
            } while (millis() < (*ptr)->Time * 1000.0 + ms);
            //} while (millis() - ms < 20000);
            System::PopRelayStack();

            Beep();
            ptr++;
        }
    }
};

class SelectTape : public Menu
{
public:
    SelectTape() : Menu(TapeInfo::Tapes::LAST_TAPE) {}
    void FullUpdate() {
        std::shared_ptr<TapeInfo> tapeInfo(TapeInfo::Get(Current));
        lcdhelper.Line(0, tapeInfo->ToString0());
        lcdhelper.Line(1, tapeInfo->ToString1());
    }
};

void NewTestTape()
{
    std::shared_ptr<TapeInfo> tapeInfo;
    {
        SelectTape selectTape;
        if (selectTape.Execute() != ButtonPanel<BasePanel>::IDOK) {
            return;
        }
        tapeInfo = std::shared_ptr<TapeInfo>(TapeInfo::Get((TapeInfo::Tapes)selectTape.Current));
    }
    if (StartRecording(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    if (AmplificationAdjustment(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
        return;
    }
    if (ValidateTapeRecorder(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
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
    if (!PrintProgress(tapeInfo.get()).Execute()) {
        return;
    }
}

void TestAllTestTape()
{
    for (TapeInfo::Tapes t = TapeInfo::Tapes::FIRST_TAPE; t != TapeInfo::Tapes::LAST_TAPE; t++) {
        std::shared_ptr<TapeInfo> tapeInfo(TapeInfo::Get(t));

        if (TapeInfo::Tapes::FIRST_TAPE == t) {
            if (StartRecording(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
                return;
            }
        }
        if (AmplificationAdjustment(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
            return;
        }
        if (ValidateTapeRecorder(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
            return;
        }
        if (RecordLevelAdjustment(tapeInfo.get()).Execute() != ButtonPanel<DialogOk>::IDOK) {
            return;
        }
    }
}

void PrintLabel()
{
    std::shared_ptr<TapeInfo> tapeInfo;
    {
        SelectTape selectTape;
        if (selectTape.Execute() != ButtonPanel<BasePanel>::IDOK) {
            return;
        }
        tapeInfo = std::shared_ptr<TapeInfo>(TapeInfo::Get((TapeInfo::Tapes)selectTape.Current));
    }
    if (!PrintProgress(tapeInfo.get()).Execute()) {
        return;
    }
}
