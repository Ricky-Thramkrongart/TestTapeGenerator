#ifndef NEWTESTTAPE_H
#define NEWTESTTAPE_H

double randomDouble(double minf, double maxf)
{
    return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
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
            std::string statuscontrol = StatusControl(0.5, LeftLevel - Target, RightLevel - Target);
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

class SelectTape : public Menu
{
    public:
        SelectTape (): Menu(TapeInfo::Tapes::LAST_TAPE) {}
        void FullUpdate() {
            std::shared_ptr<TapeInfo> tapeInfo(TapeInfo::Get(Current));
            std::vector<std::string> strs = tapeInfo->ToString();
            lcdhelper.line[0] = strs[0];
            lcdhelper.line[1] = strs[1];
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



#endif // NEWTESTTAPE_H
