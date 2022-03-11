#pragma once

#include <ArduinoSTL.h>
#include <ArxSmartPtr.h>
#include <Adafruit_Thermal.h>
#include "Dialog.h"
#include "RTCHelper.h"
#include "adaqrcode.h"
#include "SoftwareSerial.h"
#include "TapeInfo.h"
#include "TestTapeGenerator.h"
#include "SoftwareSerial.h"
#include "Controls.h"

#define TX_PIN 6 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 5 // Arduino receive   GREEN WIRE   labeled TX on printer

class PrintProgress : public Dialog
{
public:
    SoftwareSerial mySerial;
    Adafruit_Thermal printer;
    TapeInfo* tapeInfo;
    void Print(void) {
        RTC_Helper rtchelper;
        printer.justify('C');
        printer.println(TESTTAPEGENERATOR_SW_VERSION);
        printer.println(TAPELIST_VERSION);
        printer.justify('L');
        rtchelper.printlnExt(printer);
        printer.setSize('S');
        ///////tapeInfo.printlnExt(printer);

        printer.println(tapeInfo->ToString0());
        printer.println(tapeInfo->ToString1());
        for (std::vector<RecordStep*>::iterator ptr = tapeInfo->RecordSteps.begin(); ptr != tapeInfo->RecordSteps.end(); ptr++)
        {
            printer.println((*ptr)->ToStringExt().c_str());
        }

        printer.feed(4);

        //--------------------------------------------------------------------------------------------
        //  QR Kode
        //----------------------------------------
        // Print the 135x135 pixel QR code in adaqrcode.h:
        printer.setDefault(); // Restore printer to defaults
        printer.justify('C');
        printer.printBitmap(adaqrcode_width, adaqrcode_height, adaqrcode_data);
        printer.feed(2);
    }
    PrintProgress(TapeInfo* tapeInfo_) : Dialog(1000), tapeInfo(tapeInfo_), mySerial(RX_PIN, TX_PIN), printer(&mySerial)
    {
        mySerial.begin(9600);  // Initialize SoftwareSerial til 9600 ( printer)
        printer.begin();        // Init printer (same regardless of serial type)
    }
    void FullUpdate() {
        lcdhelper.Line(0, F("Printing"));
        lcdhelper.Line(1, tapeInfo->ToString0());
        lcdhelper.Line(2, tapeInfo->ToString1());
        lcdhelper.Show();
        if (!finished) {
            Print();
            lcdhelper.Line(0, F("Finished Printing"));
            finished = true;
        }
    }
};