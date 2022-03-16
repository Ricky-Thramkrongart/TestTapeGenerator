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
        bool Sweep;
        static void pad(SafeString& s, const int n)
        {
            if (n < 10) {
                s.print(F("0"));
            }
            s.print(n);
        }

        void Print(void) {
            printer.setDefault();
            printer.println("*******************************");
            printer.boldOn();
            printer.println("Calibration/Test-Tape    type:");
            printer.boldOff();
            printer.println("................................");

            printer.justify('L');
            printer.setSize('S');
            ///////tapeInfo.printlnExt(printer);
            printer.boldOn();
            printer.justify('C');
            printer.println(tapeInfo->ToString0());
            printer.boldOff();
            printer.justify('L');
            printer.println("-------------------------------");
            printer.println(tapeInfo->ToString1(true));
            printer.println("-------------------------------");
            for (std::vector<RecordStep*>::iterator ptr = tapeInfo->RecordSteps.begin(); ptr != tapeInfo->RecordSteps.end(); ptr++)
            {
                printer.print(F("Track#: ")); printer.println(ptr - tapeInfo->RecordSteps.begin() + 1);
                if ((*ptr)->Comment) {
                    printer.println((*ptr)->Comment);
                }
                printer.println((*ptr)->ToString().c_str());
                if (Sweep) {
                    printer.print("Record Level: ("); 
                    printer.print(tapeInfo->RecordLevels[ptr - tapeInfo->RecordSteps.begin()].first - (*ptr)->Level - tapeInfo->GetAmplificationAdjustment());
                    printer.print(",");
                    printer.print(tapeInfo->RecordLevels[ptr - tapeInfo->RecordSteps.begin()].second - (*ptr)->Level - tapeInfo->GetAmplificationAdjustment());
                    printer.println(")dBm");
                }
                printer.println();
            }
            printer.feed(2);
            printer.justify('L');
            printer.println("*******************************");
            //   printer.justify('C');
            printer.println(TESTTAPEGENERATOR_SW_VERSION);
            printer.println(TAPELIST_VERSION);
            printer.justify('L');
            printer.println("*******************************");
            printer.println("===============================");
            printer.justify('C');
            printer.doubleHeightOn();
            printer.setSize('L');
            printer.println("QC PASSED");
            
            // YYYY/MM/DD-HH:MM:SS
            cSF(sf_line, 100);
            pad(sf_line, year());
            sf_line.print(F("/"));
            pad(sf_line, month());
            sf_line.print(F("/"));
            pad(sf_line, day());
            sf_line.print(F("-"));
            pad(sf_line, hour());
            sf_line.print(F(":"));
            pad(sf_line, minute());
            printer.println(sf_line);

            printer.doubleHeightOff();
            printer.setSize('S');
            printer.justify('L');
            printer.println("===============================");
            printer.feed(2);

            //--------------------------------------------------------------------------------------------
            //  QR Kode
            //----------------------------------------
            // Print the 135x135 pixel QR code in adaqrcode.h:
            printer.setDefault(); // Restore printer to defaults
            printer.justify('C');
            printer.printBitmap(adaqrcode_width, adaqrcode_height, adaqrcode_data);
            printer.feed(1);

            printer.println("*******************************");
            printer.println("The Reel Deal Denmark");
            printer.println("High quality calibration tapes");
            printer.println("Find my Ebay store by scanning");
            printer.println("The QR code above");
            printer.println("*******************************");
            printer.feed(3);


        }
        PrintProgress(TapeInfo* tapeInfo_, bool Sweep_=false) : Dialog(1000), tapeInfo(tapeInfo_), mySerial(RX_PIN, TX_PIN), printer(&mySerial), Sweep(Sweep_)
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
