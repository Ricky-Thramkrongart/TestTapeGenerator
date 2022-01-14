#ifndef PRINTER_H
#define PRINTER_H
//-------------------------------------------------------------------------
// TIL PRINTER
//-------------------------------------------------------------------------
#include "adaqrcode.h"
#include <Adafruit_Thermal.h>
#include "SoftwareSerial.h"
#include "TapeInfo.h"
#include "TestTapeGenerator.h"
#include "SoftwareSerial.h"
#include "Controls.h"
#include <RTC.h>
#define RTC_H //Bug in RTC.h

#define TX_PIN 6 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 5 // Arduino receive   GREEN WIRE   labeled TX on printer

class PrintProgress : public Dialog
{
    public:
        SoftwareSerial mySerial;
        Adafruit_Thermal printer;
        std::shared_ptr<TapeInfo> tapeInfo;
        void Print(void) {
            printer.justify('C');
            printer.println(TESTTAPEGENERATOR_SW_VERSION);
            printer.println(TAPELIST_VERSION);
            printer.justify('L');
            printer.println((RTC_Helper().ToStringExt().c_str()));
            printer.setSize('S');
            printer.println(tapeInfo->ToString()[0].c_str());
            printer.println(tapeInfo->ToString()[1].c_str());
            for (std::vector<RecordStep*>::iterator ptr = tapeInfo->RecordSteps.begin() ; ptr != tapeInfo->RecordSteps.end(); ptr++)
            {
                if ((*ptr)->Comment.size()) {
                    printer.println((*ptr)->ToStringExt().c_str());
                }
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
        PrintProgress(TapeInfo::Tapes Tape): Dialog(1000),  tapeInfo(TapeInfo::Get(Tape)), mySerial(RX_PIN, TX_PIN), printer(&mySerial)
        {
            mySerial.begin(9600);  // Initialize SoftwareSerial til 9600 ( printer)
            printer.begin();        // Init printer (same regardless of serial type)
        }
        void FullUpdate() {
            lcdhelper.line[0] = "Printing";
            lcdhelper.line[1] = tapeInfo->ToString()[0];
            lcdhelper.line[2] = tapeInfo->ToString()[1];
            lcdhelper.Show();
            if (!finished) {
                Print();
                lcdhelper.line[0] = "Finished Printing";
                finished = true;
            }
        }
};

#endif // PRINTER_H
