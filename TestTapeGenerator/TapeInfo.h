#pragma once

#include <ArduinoSTL.h>
#include <SafeString.h>

const char* TAPELIST_VERSION = "TapeList: " __DATE__ " " __TIME__;

class RecordStep
{
public:
    int Frequency;
    int Time;
    int Level;
    const __FlashStringHelper* Comment;
    RecordStep(int Frequency_, int Time_, int Level_, const __FlashStringHelper* Comment_ = 0) : Frequency(Frequency_), Time(Time_), Level(Level_), Comment(Comment_)
    {
    }

    std::string ToString()
    {
        cSF(sf_line, 100);
        sf_line.print(Frequency, 5);
        sf_line.print(F("Hz "));
        sf_line.print(Level, 5);
        sf_line.print(F("dBm "));
        sf_line.print(Time, 5);
        sf_line.print(F("s"));
        return sf_line.c_str();
    }

    std::string ToStringExt()
    {
        cSF(sf_line, 100);
        sf_line = ToString().c_str();
        sf_line.print(F(" "));
        sf_line.print(Comment);
        return sf_line.c_str();
    }

    void println(Stream& out)
    {
        out.println(ToString().c_str());
    }
};

class TapeInfo
{
public:
    enum TapeFormat { Cassette, Reel };
    const __FlashStringHelper* Description;
    uint16_t Tracks;
    uint16_t Length;
    uint16_t Flux;
    TapeFormat Format;
    double Target;
    std::vector<RecordStep*> RecordSteps;

    TapeInfo(
        const __FlashStringHelper* Description_,
        uint16_t Tracks_,
        uint16_t Flux_,
        TapeFormat Format_,
        double Target_,
        std::vector<RecordStep*> RecordSteps_) : Description(Description_), Tracks(Tracks_), Flux(Flux_), Format(Format_), Target(Target_), RecordSteps(RecordSteps_)
    {
        Length = 0;
        for (std::vector<RecordStep*>::iterator ptr = RecordSteps.begin(); ptr < RecordSteps.end(); ptr++) {
            Length += (*ptr)->Time;
        }
    }
    ~TapeInfo() {
        std::vector<RecordStep*>::iterator ptr;
        for (std::vector<RecordStep*>::iterator ptr = RecordSteps.begin(); ptr < RecordSteps.end(); ptr++) {
            delete* ptr;
        }
    }

    std::vector<std::string> ToString()
    {
        cSF(sf_line, 41);
        std::vector<std::string> Result(2);
        char format = 'R';
        if (Format == TapeFormat::Cassette) {
            format = 'C';
        }
        sf_line.print(F("Track:")); sf_line.print(Tracks);
        sf_line.print(F(" Tracks:"));  sf_line.print(RecordSteps.size()); sf_line.print(F(" "));
        sf_line.print((Length % 60) ? Length / 60 + 1 : Length / 60); sf_line.print(F("[Min] ")); 
        sf_line.print(Flux); sf_line.print(F("[nW/m] ")); 
        sf_line.print(format);
        Result[0] = String(Description).c_str();
        Result[1] = sf_line.c_str();
        return Result;
    }


    enum Tapes {
        FIRST_TAPE,
        AKAI_GX_75_95_TEST_TAPE = FIRST_TAPE,
        NAKAMICHI_TEST_TAPE,
        PANASONIC_QZZCFM_TEST_TAPE,
        PLAYBACK_EQ_TEST_TAPE,
        REVOX_B215_TEST_TAPE,
        REVOX_B77_TEST_TAPE_1,
        REVOX_B77_TEST_TAPE_2,
        STUDER_A710_TEST_TAPE,
        TANDBERG_24_TEST_TAPE,
        TEAC_MTT_MULTI_TEST_TAPE,
        UNIVERSAL_TEST_TAPE,
        WOW_AND_FLUTTER_TEST_TAPE_1,
        WOW_AND_FLUTTER_TEST_TAPE_2,
        WOW_AND_FLUTTER_TEST_TAPE_3,
        WOW_AND_FLUTTER_TEST_TAPE_4,
        LAST_TAPE
    };
    static Tapes Begin()
    {
        return FIRST_TAPE;
    };
    static Tapes End()
    {
        return LAST_TAPE;
    };

    static TapeInfo* Get(Tapes tape) {
        switch (tape) {
        case AKAI_GX_75_95_TEST_TAPE:
            return new TapeInfo(F("AKAI GX-75/95 Test Tape"), 2, 200, TapeInfo::Cassette, 0.0, {
                new RecordStep(315, 180, 0, F("Level")),
                new RecordStep(1000, 180, 0, F("Azimuth")),
                new RecordStep(3150, 180, 0),
                new RecordStep(10000, 180, -15)
                });
        case NAKAMICHI_TEST_TAPE:
            return new TapeInfo(F("Nakamichi Test Tape"), 2, 200, TapeInfo::Cassette, 0.0, {
                new RecordStep(20000, 180, -20, F("DA09001A 20 kHz Frequency Response")),
                new RecordStep(15000, 180, -20, F("DA09002A 15 kHz Frequency Response")),
                new RecordStep(10000, 180, -20, F("DA09003A 10 kHz Frequency Response")),
                new RecordStep(15000, 180, 0, F("DA09004A 15 kHz Azimuth")),
                new RecordStep(400, 180, -10, F("DA09005A 400 Hz Playback Level")),
                new RecordStep(3000, 180, 0, F("DA09006A 3 kHz Speed and Wow & Flutter"))
                });
        case PANASONIC_QZZCFM_TEST_TAPE:
            return new TapeInfo(F("Panasonic QZZCFM Test Tape"), 2, 200, TapeInfo::Cassette, 0.0, {
                new RecordStep(315, 180, -2, F("Level")),
                new RecordStep(3000, 180, -10, F("Azimuth")),
                new RecordStep(8000, 180, -20),
                new RecordStep(63, 20, -20),
                new RecordStep(125, 20, -20),
                new RecordStep(250, 20, -20),
                new RecordStep(1000, 20, -20),
                new RecordStep(2000, 20, -20),
                new RecordStep(8000, 20, -20),
                new RecordStep(10000, 20, -20),
                new RecordStep(12500, 20, -20)
                });
        case PLAYBACK_EQ_TEST_TAPE:
            return new TapeInfo(F("Playback EQ Test Tape"), 2, 200, TapeInfo::Cassette, 0.0, {
                //new RecordStep(1000, 120, 0, "1 kHz Reference Level"),
                //new RecordStep(10000, 5, -20, "Alternating 10kHz"),
                //new RecordStep(1000, 5, -20, "Alternating 1kHz"),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 5, -20),
                new RecordStep(1000, 5, -20),
                new RecordStep(10000, 120, -20, F("Azimuth"))
                });
        case REVOX_B215_TEST_TAPE:
            return   new TapeInfo(F("Revox B-215 Test Tape"), 2, 200, TapeInfo::Cassette, 0.0, {
                new RecordStep(1000, 180, 2),
                new RecordStep(1000, 180, 0),
                new RecordStep(3150, 180, 0),
                new RecordStep(10000, 180, -20),
                new RecordStep(16000, 180, -20)
                });
        case REVOX_B77_TEST_TAPE_1:
            return new TapeInfo(F("Revox B-77 Test Tape #1"), 2, 257, TapeInfo::Reel, 0.0, {
                new RecordStep(1000, 120, 0, F("Level")),
                new RecordStep(1000, 60, -20, F("Azimuth")),
                new RecordStep(10000, 120, -20, F("Azimuth")),
                new RecordStep(10000, 120, 0, F("Azimuth"))
                });
        case REVOX_B77_TEST_TAPE_2:
            return new TapeInfo(F("Revox B-77 Test Tape #2"), 4, 257, TapeInfo::Reel, 0.0, {
                new RecordStep(1000, 120, 0, F("Level")),
                new RecordStep(1000, 60, -20, F("Azimuth")),
                new RecordStep(10000, 120, -20, F("Azimuth")),
                new RecordStep(10000, 120, 0, F("Azimuth"))
                });
        case STUDER_A710_TEST_TAPE:
            return new TapeInfo(F("Studer A710 Test Tape"), 2, 200, TapeInfo::Cassette, 0.0, {
                new RecordStep(315, 460, 0, F("Level")),
                new RecordStep(10000, 460, -20, F("Azimuth"))
                });
        case TANDBERG_24_TEST_TAPE:
            return new TapeInfo(F("Tandberg #24 Test Tape"), 2, 250, TapeInfo::Cassette, 0.0, {
                new RecordStep(1000, 4800, +2, F("Flux: 250nW/m"))
                });
        case TEAC_MTT_MULTI_TEST_TAPE:
            return new TapeInfo(F("Tandberg #24 Test Tape"), 2, 250, TapeInfo::Cassette, 0.0, {
                new RecordStep(315, 120, 0, F("MTT-212N")),
                new RecordStep(315, 120, -4, F("MTT-212CN")),
                new RecordStep(1000, 120, -4, F("MTT-212EN")),
                new RecordStep(1000, 120, -10, F("MTT-118N")),
                new RecordStep(3000, 120, -10, F("MTT-111N")),
                new RecordStep(6300, 120, -10, F("MTT-113N")),
                new RecordStep(8000, 120, -10, F("MTT-113CN")),
                new RecordStep(10000, 120, -10, F("MTT-114N")),
                new RecordStep(12500, 120, -24, F("MTT-118NA"))
                });
        case UNIVERSAL_TEST_TAPE:
            return new TapeInfo(F("Universal Test Tape"), 2, 200, TapeInfo::Cassette, 0.0, {
                new RecordStep(1000, 120, 0, F("Level")),
                new RecordStep(31.5, 60, 0, F("Azimuth")),
                new RecordStep(31.5, 60, -3),
                new RecordStep(31.5, 60, -6),
                new RecordStep(31.5, 60, -10),
                new RecordStep(31.5, 60, -15),
                new RecordStep(31.5, 60, -20),
                new RecordStep(63, 60, 0),
                new RecordStep(63, 60, -3),
                new RecordStep(63, 60, -6),
                new RecordStep(63, 60, -10),
                new RecordStep(63, 60, -15),
                new RecordStep(63, 60, -20),
                new RecordStep(125, 60, 0),
                new RecordStep(125, 60, -3),
                new RecordStep(125, 60, -6),
                new RecordStep(125, 60, -10),
                new RecordStep(125, 60, -15),
                new RecordStep(125, 60, -20),
                new RecordStep(250, 60, 0),
                new RecordStep(250, 60, -3),
                new RecordStep(250, 60, -6),
                new RecordStep(250, 60, -10),
                new RecordStep(250, 60, -15),
                new RecordStep(250, 60, -20),
                new RecordStep(500, 60, 0),
                new RecordStep(500, 60, -3),
                new RecordStep(500, 60, -6),
                new RecordStep(500, 60, -10),
                new RecordStep(500, 60, -15),
                new RecordStep(500, 60, -20),
                new RecordStep(1000, 60, 0),
                new RecordStep(1000, 60, -3),
                new RecordStep(1000, 60, -6),
                new RecordStep(1000, 60, -10),
                new RecordStep(1000, 60, -15),
                new RecordStep(1000, 60, -20),
                new RecordStep(2000, 60, 0),
                new RecordStep(2000, 60, -3),
                new RecordStep(2000, 60, -6),
                new RecordStep(2000, 60, -10),
                new RecordStep(2000, 60, -15),
                new RecordStep(2000, 60, -20),
                new RecordStep(4000, 60, 0),
                new RecordStep(4000, 60, -3),
                new RecordStep(4000, 60, -6),
                new RecordStep(4000, 60, -10),
                new RecordStep(4000, 60, -15),
                new RecordStep(4000, 60, -20),
                new RecordStep(6300, 60, 0),
                new RecordStep(6300, 60, -3),
                new RecordStep(6300, 60, -6),
                new RecordStep(6300, 60, -10),
                new RecordStep(6300, 60, -15),
                new RecordStep(6300, 60, -20),
                new RecordStep(8000, 60, 0),
                new RecordStep(8000, 60, -3),
                new RecordStep(8000, 60, -6),
                new RecordStep(8000, 60, -10),
                new RecordStep(8000, 60, -15),
                new RecordStep(8000, 60, -20),
                new RecordStep(10000, 60, -10),
                new RecordStep(10000, 60, -15),
                new RecordStep(10000, 60, -20),
                new RecordStep(12500, 60, -10),
                new RecordStep(12500, 60, -15),
                new RecordStep(12500, 60, -20),
                new RecordStep(15000, 60, -10),
                new RecordStep(15000, 60, -15),
                new RecordStep(15000, 60, -20),
                new RecordStep(1000, 120, 0)
                });
        case WOW_AND_FLUTTER_TEST_TAPE_1:
            return new TapeInfo(F("Wow & Flutter Test Tape #1"), 2, 200, TapeInfo::Cassette, 0.0, {
                new RecordStep(3000, 120, 0, F("DIN Standard")),
                new RecordStep(3150, 120, 0, F("JIS Standard"))
                });
        case WOW_AND_FLUTTER_TEST_TAPE_2:
            return new TapeInfo(F("Wow & Flutter Test Tape #2"), 2, 257, TapeInfo::Reel, 0.0, {
                new RecordStep(3000, 120, 0, F("DIN Standard")),
                new RecordStep(3150, 120, 0, F("JIS Standard"))
                });
        case WOW_AND_FLUTTER_TEST_TAPE_3:
            return new TapeInfo(F("Wow & Flutter Test Tape #3"), 4, 185, TapeInfo::Reel, -5.0, {
                new RecordStep(3000, 120, 0, F("DIN Standard")),
                new RecordStep(3150, 120, 0, F("JIS Standard"))
                });
        case WOW_AND_FLUTTER_TEST_TAPE_4:
            return new TapeInfo(F("Wow & Flutter Test Tape #4"), 4, 257, TapeInfo::Reel, 0.0, {
                new RecordStep(3000, 120, 0, F("DIN Standard")),
                new RecordStep(3150, 120, 0, F("JIS Standard"))
                });
        default:
            break;
        };
    };
};

TapeInfo::Tapes& operator++(TapeInfo::Tapes& t, int)
{
    if (t + 1 < TapeInfo::End()) {
        return t = t + 1;
    }
    return t;
}

TapeInfo::Tapes& operator--(TapeInfo::Tapes& t, int)
{
    if (t - 1 >= TapeInfo::Begin()) {
        return t = t - 1;
    }
    return t;
}