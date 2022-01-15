//**********************************************************************
// Calibration tape generator box  Niels 2017
//**********************************************************************
// HUSK at PT2257 lib kun kan køre med ver 1.6.9 eller lavere
/*
  The connections to the AD9837 board are:

  FSYNC -> 2
  SCLK -> 13 (SCK)
  SDATA -> 11 (MOSI)
  MCLK -> 9 (Timer1)
*/

long freq; //32-bit global frequency variable

#include <SPI.h>
#include "TimerOne.h"

#define FSYNC 2                // Define the FSYNC (used for SD funtion på DDS)
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Evc_pt2257.h>        // attanuator PT2257 biblotek
LiquidCrystal_I2C lcd(0x27, 16, 4); // Set the LCD address to 0x27 for a 16 chars and 4 line display


const int buttonsel = 3;       // the number of the SELECT pushbutton pin
const int buttonstart = 4;     // the number of the START pushbutton pin
const int buttonstop = 5;      // the number of the STOP pushbutton pin
const int buttoncal = 6;       // the number of the SCAL pushbutton pin
int LR = 9;                    // the number of the Left/Right skifte pin
const int analogPin = A3 ;     // Analog input pin på A3
int Sel = 0;                   // register for Select knap
int Start = 0;                 // register for Start knap
int Stop = 0;                  // register for Stop knap
int Cal = 0;                   // register for Cal knap
int Case = 0;                  // register for case tæller
float valA = 0 ;
float valB = 0 ;
int attlev = 0 ;
int setlev = 0 ;

int AMP = 0;
int FREQ = 0;
int TIME = 0;
int TID = 0;
int OFFSET = 0;
int OFFSET1 = 0;
int POSR = 0;
int POSL = 0;
//***********************************************************************
void setup()
{
  pinMode(buttonsel, INPUT);    // initialize the pushbutton pin as an input:
  pinMode(buttonstart, INPUT);  // initialize the pushbutton pin as an input:
  pinMode(buttonstop, INPUT);   // initialize the pushbutton pin as an input:
  pinMode(buttoncal, INPUT);    // initialize the pushbutton pin as an input:
  pinMode(LR, OUTPUT);          // set LR pin som output
  lcd.begin();                  // initialize the LCD
  delay(100);                   // vent
  evc_init();                   // init af PT2257
  delay(100);                   // vent
  //******************************************************************
  // setup for AD9833
  //******************************************************************
  Timer1.initialize(1);
  Timer1.pwm(9, 512);
  pinMode(FSYNC, OUTPUT);      //FSYNC signal
  digitalWrite(FSYNC, HIGH);   // sæt FSYNC signal høj
  SPI.setDataMode(SPI_MODE2);  // requires SPI Mode for AD9837
  SPI.begin();
  delay(100);                  // vent
  evc_mute(true);              // mute både output og input til -95dB
  //********************************************************************
  //           Splash screen
  //********************************************************************
  lcd.backlight();
  lcd.setCursor(2, 0);            //  set position og linje
  lcd.print("Calibration Tape");
  lcd.setCursor(6, 1);            //  sæt position og linje
  lcd.print("Generator");
  lcd.setCursor(4, 2);            //  sæt position og linje
  lcd.print("SW 2017-10-25");
  lcd.setCursor(6, 3);            //  sæt position og linje
  lcd.print("NSL 2017");
  delay(2000);                    // vent 2 sec og skift MENU skærm
  //********************************************************************
  //    MENU skærm
  //********************************************************************
  lcd.clear();                    // clear the screen
  lcd.backlight();                //
  lcd.setCursor(0, 0);            //  position og linje
  lcd.print(">Select  tape model<");
  lcd.setCursor(1, 1);            //  sæt position og linje
  lcd.print("Or Calibration mode");
  lcd.setCursor(8, 2);            //  sæt position og linje
  lcd.print("Press:");
  lcd.setCursor(0, 3);            //  sæt position og linje
  lcd.print("SELECT   or      CAL");
}
//*********************************************************************
void loop()
{
  Sel = digitalRead(buttonsel);       // læs status på start knap
  Start = digitalRead(buttonstart);   // læs status på start knap
  Stop = digitalRead(buttonstop);     // læs status på stop knap
  Cal = digitalRead(buttoncal);       // læs status på cal knap
  if (Sel == HIGH) {                  // er SELECT knap aktiv
    Case = Case + 1;                  // så læg 1 til case tæller
    delay(200);                       // prell filter
  }
  if (Case > 13) {                    // er case større end 13 rulles til bage på case 0
    Case = 0;
  }
  if (Stop == HIGH) {                  // er STOP aktiv Så
    asm volatile ("  jmp 0");         // lav en softstart
  }
  //***************************************************************
  //    Start CAL RUTINE
  //***************************************************************
  if (Cal == HIGH) {                   // er CAL knap aktiv så
    lcd.clear();                      // clear the screen
    lcd.setCursor(1, 0);              //  position og linje
    lcd.print("Calibration mode");    //  Skriv i display
    lcd.setCursor(3, 1);              //  position og linje
    lcd.print("1 kHz / 0 dBm");       //  Skriv i display
    //**************************************************************
    //  set frekvens og nivau for kalibrerings rutine
    //**************************************************************
    freq = 1000;                      // set cal frekvens til 1 kHz
    WriteFrequencyAD9837(freq);       // skriv til DDS
    setlev = 0;                       // sæt nivau til 0dBm
    //**************************************************************
    attlev = 30 - setlev ;            // set input attenuator ofset 30dB under output lev
    evc_setVolumeRight(attlev);       // set attanuator for input
    delay(100);                       // vent
    evc_setVolumeLeft(setlev) ;       // set nivau for output
    delay(100);                       // vent
    evc_mute(false);                  // tænd for signalet

    // ****************************************************
    //læs kanaler og vis i display
    //*****************************************************
    for (int i = 0; i <= 255; i++) {    // timer for cal signal
      //******************************************************
      // Læs analog input for højre kanal
      //******************************************************
      Stop = digitalRead(buttonstop);   // læs status på stop knap
      digitalWrite(LR, HIGH);           // skift til højre kanal
      if (Stop == HIGH) {                // er stop aktiv
        asm volatile ("  jmp 0");       // lav en softstart
      }
      delay(500);                       // vent
      valA = (7 * log(((analogRead(analogPin) - 6))) - 30 );    // gem højre kanal i valA
      lcd.setCursor(10, 2);             //  position og linje
      lcd.print("     ");
      lcd.setCursor(1, 2);              //  position og linje
      lcd.print("Input R :");
      lcd.print(valA, 1);
      lcd.setCursor(16, 2);             //  position og linje
      lcd.print("dBm");


      //******************************************************
      // Læs analog input for venstre kanal
      //******************************************************
      Stop = digitalRead(buttonstop);   // læs status på stop knap
      digitalWrite(LR, LOW);            // skift til venstre kanal
      if (Stop == HIGH) {                // er stop aktiv
        asm volatile ("  jmp 0");       // lav en softstart
      }
      delay(500);                       // vent
      valB = (7 * log(((analogRead(analogPin) - 6))) - 30 ); // gem venstre kanal i valB
      lcd.setCursor(10, 3);             //  position og linje
      lcd.print("     ");
      lcd.setCursor(1, 3);              //  position og linje
      lcd.print("Input L :");
      lcd.print(valB, 1);               // skriv i display med 1 decimaler
      lcd.setCursor(16, 3);             //  position og linje
      lcd.print("dBm");

    }
    //********************************************************************
    //   Kalibrering TIME OUT
    //********************************************************************
    lcd.clear();                       // clear the screen
    lcd.setCursor(6, 1);               //  position og linje
    lcd.print("Finish");               // Skriv i LCD
    delay(2000);                       // vent 2 sec og
    asm volatile ("  jmp 0");          // lav en softstart når tiden løber ud
  }

  //***********************************************************************
  //#######################################################################
  //   HER UNDER ER DE ENKELTE SEKVENSER DIFINERET
  //#######################################################################
  //***********************************************************************
  switch (Case) {

    case 1:
      //*******************************************************************
      //   REVOX B-77
      //*******************************************************************
      lcd.setCursor(0, 0);                //  position og linje
      lcd.print("REVOX B-77 Test tape");
      lcd.setCursor(0, 1);                //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 2);                //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3);                //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {                  // er stop aktiv
        asm volatile ("  jmp 0");         // lav en softstart
      }
      if (Start == HIGH) {                 // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 1000 ;
        TIME = 120 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 1000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 10000 ;
        TIME = 120 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 10000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

    case 2:
      //*******************************************************************
      //   REEL TO REEL  WOW & FLUTTER TAPE
      //*******************************************************************

      lcd.setCursor(0, 0); //  position og linje
      lcd.print("Wow&Flutter Testtape");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("    Reel to Reel    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 3000 ;
        TIME = 120 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 3150 ;
        TIME = 120 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

    case 3:
      //*******************************************************************
      //   AKAI GX-75 / 95 test tape
      //*******************************************************************
      lcd.setCursor(0, 0); //  position og linje
      lcd.print("AKAI GX-75 Test tape");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 315 ;
        TIME = 180 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 1000 ;
        TIME = 180 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 3150 ;
        TIME = 180 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 10000 ;
        TIME = 180 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

    case 4:
      //*******************************************************************
      //   Nakamichi dragon  test tape
      //*******************************************************************
      lcd.setCursor(0, 0); //  position og linje
      lcd.print(" Nakamichi Testtape ");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 20000 ;
        TIME = 180 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 15000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 10000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 0;
        FREQ = 400 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 3000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 0;
        FREQ = 1000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

    case 5:
      //*******************************************************************
      //   Universal  test tape casette
      //*******************************************************************
      lcd.setCursor(0, 0); //  position og linje
      lcd.print("Universal Test tape ");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("   Cassette deck    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 100 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 3 ;
        FREQ = 100 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 6 ;
        FREQ = 100 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 100 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 100 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 100 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 400 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 3 ;
        FREQ = 400 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 6 ;
        FREQ = 400 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 400 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 400 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 400 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 1000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 3 ;
        FREQ = 1000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 6 ;
        FREQ = 1000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 1000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 1000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 1000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 3000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 3 ;
        FREQ = 3000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 6 ;
        FREQ = 3000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 3000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 3000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 3000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 10000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 3 ;
        FREQ = 10000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 6 ;
        FREQ = 10000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 10000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 10000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 10000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //-------------------------------------------------------------------
        AMP = 10 ;
        FREQ = 15000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15 ;
        FREQ = 15000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 15000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 20000 ;
        TIME = 60 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

    case 6:
      //*******************************************************************
      //   AKAI GX-F66R  test tape
      //*******************************************************************
      lcd.setCursor(0, 0); //  position og linje
      lcd.print("AKAI  GX-F66R  tape ");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 5 ;
        FREQ = 333 ;
        TIME = 180 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 1000 ;
        TIME = 180 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 3150 ;
        TIME = 180 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 19;
        FREQ = 10000 ;
        TIME = 180 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

    case 7:
      //*******************************************************************
      //   Frequency responce  test tape casette R2R
      //*******************************************************************
      lcd.setCursor(0, 0); //  position og linje
      lcd.print("Freq resp. Testtape ");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("    Reel to Reel    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 100 ;
        TIME = 10 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 3 ;
        FREQ = 100 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 6 ;
        FREQ = 100 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 100 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 100 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 100 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 400 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 3 ;
        FREQ = 400 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 6 ;
        FREQ = 400 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 400 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 400 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 400 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 1000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 3 ;
        FREQ = 1000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 6 ;
        FREQ = 1000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 1000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 1000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 1000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 3000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 3 ;
        FREQ = 3000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 6 ;
        FREQ = 3000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 3000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 3000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 3000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 10000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 3 ;
        FREQ = 10000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 6 ;
        FREQ = 10000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10;
        FREQ = 10000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15;
        FREQ = 10000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 10000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //-------------------------------------------------------------------
        AMP = 10 ;
        FREQ = 15000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 15 ;
        FREQ = 15000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 15000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        delay(3000); // 3 sek pause
        //--------------------------------------------------------------------
        AMP = 20;
        FREQ = 20000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

    case 8:
      //*******************************************************************
      //   Revox b215  test tape
      //*******************************************************************

      lcd.setCursor(0, 0); //  position og linje
      lcd.print("REVOX B-215 Testtape");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("  Adjust cal to +2  ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 1000 ;
        TIME = 120 ;
        OFFSET = 2 ; // offset til visning af setnivau
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 2 ;
        FREQ = 1000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 2 ;
        FREQ = 3150 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 22;
        FREQ = 10000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 22;
        FREQ = 16000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

    case 9:
      //*******************************************************************
      //   Cassette  WOW & FLUTTER TAPE
      //*******************************************************************

      lcd.setCursor(0, 0); //  position og linje
      lcd.print("Wow&Flutter Testtape");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("   Cassette deck    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 3000 ;
        TIME = 480 ;   // ( 8 minutter )
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 3150 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

    case 10:
      //*******************************************************************
      //   PANASONIC QZZCFM TEST TAPE
      //*******************************************************************

      lcd.setCursor(0, 0); //  position og linje
      lcd.print("QZZCFM     Test tape");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("   Cassette deck    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen

        //-------------------------------------------------------------------
        AMP = 2 ;
        FREQ = 315 ;
        TIME = 180 ;   // ( 8 minutter )
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 10 ;
        FREQ = 3000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 8000;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 63 ;
        TIME = 20;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 125 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 250 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 1000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 4000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 8000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 10000 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 12500 ;
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

    case 11:
      //*******************************************************************
      //   STUDER A710 Testtape
      //*******************************************************************

      lcd.setCursor(0, 0); //  position og linje
      lcd.print("STUDER A710 Testtape");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("   Cassette deck    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 315 ;
        TIME = 460 ;   //
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 10000 ;
        TIME = 460 ;   //
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

        case 12:
      //*******************************************************************
      //   AZIMUTH    TEST TAPE   20 min
      //*******************************************************************

      lcd.setCursor(0, 0); //  position og linje
      lcd.print("AZIMUTH    TEST TAPE");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("   Cassette deck    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 20 ;
        FREQ = 10000 ;
        TIME = 1200 ;   //  kør i 20 min
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
      }
      break;

        case 13:
      //*******************************************************************
      //   Spare 13
      //*******************************************************************

      lcd.setCursor(0, 0); //  position og linje
      lcd.print("SPARE 13");
      lcd.setCursor(0, 1); //  position og linje
      lcd.print("   Cassette deck    ");
      lcd.setCursor(0, 2); //  position og linje
      lcd.print("                    ");
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("START    STOP    CAL");
      delay(200);  //
      if (Stop == HIGH) {  // er stop aktiv
        asm volatile ("  jmp 0"); // lav en softstart
      }
      if (Start == HIGH) {  // er start aktiv så kør sekvensen
        //-------------------------------------------------------------------
        AMP = 0 ;
        FREQ = 315 ;
        TIME = 480 ;   //
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------
        AMP = 2 ;
        FREQ = 10000 ;
        TIME = 480 ;   //
        setpar();   // kald funktion til at sætte frekvens / amplitude / tid
        //--------------------------------------------------------------------

      }
      break;
      
    //*******************************************************************
    //break;
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
  }
}
void WriteFrequencyAD9837(long frequency)
{
  int MSB;
  int LSB;
  int phase = 0;

  //We can't just send the actual frequency, we have to calculate the "frequency word".
  //This amounts to ((desired frequency)/(reference frequency)) x 0x10000000.
  //calculated_freq_word will hold the calculated result.
  long calculated_freq_word;
  float AD9837Val = 0.00000000;

  AD9837Val = (((float)(frequency)) / 25000000);  // 25 MHz internal clock på ad9833
  calculated_freq_word = AD9837Val * 0x10000000;

  //Split det op i to separate bytes.
  MSB = (int)((calculated_freq_word & 0xFFFC000) >> 14); //14 bits
  LSB = (int)(calculated_freq_word & 0x3FFF);

  //Set control bits DB15 ande DB14 to 0 and one, respectively, for frequency register 0
  LSB |= 0x4000;
  MSB |= 0x4000;

  phase &= 0xC000;

  WriteRegisterAD9837(0x2100);

  //delay(500);

  //*****************Set the frequency********************************
  WriteRegisterAD9837(LSB); //lower 14 bits

  WriteRegisterAD9837(MSB); //upper 14 bits

  WriteRegisterAD9837(phase); //mid-low

  //Power it back up
  //*****************************
  // HER VÆLGES SIGNAL TYPE SIN
  //*****************************
  //AD9837Write(0x2020); //square
  WriteRegisterAD9837(0x2000); //sinus signal
  //AD9837Write(0x2002); //triangle

}
//***************************************************************
//This is the FUNCTION that does the actual talking to the AD9837
//***************************************************************
void WriteRegisterAD9837(int dat)
{
  digitalWrite(FSYNC, LOW); //Set FSYNC low
  delay(10);

  SPI.transfer(highByte(dat)); Serial.println(highByte(dat));
  SPI.transfer(lowByte(dat)); Serial.println(lowByte(dat));

  delay(10);
  digitalWrite(FSYNC, HIGH); //Set FSYNC high
}

//**********************************************************************************
// Denne funktion sætter frekvens  nivau og tid og udlæser i disp
//**********************************************************************************
void setpar()
{
  freq = FREQ;  // set  frekvens
  WriteFrequencyAD9837(freq);
  setlev = AMP;  // sæt nivau som AMP
  //*********************************************
  //attlev = 0 ;
  attlev = (30 - setlev) ;     // set attenuator ofset 30dB under output lev
  TID = ( TIME * 0.92);       // skalering af timebase
  evc_setVolumeRight(attlev);  //set attanuator for input
  delay(100);                  // vent
  evc_setVolumeLeft(setlev) ;  //set nivau for output
  delay(100);                  // vent
  evc_mute(false);             // tænd for signalet

  lcd.clear();                 // clear the screen
  lcd.setCursor(2, 0);         //  position og linje
  lcd.print(FREQ);             // skriv frekvensen i LCD
  lcd.print(" Hz");
  lcd.print(" / -");
  lcd.print(AMP - OFFSET);
  lcd.print(" dBm");

  for (int x = 0; x <= TID; x++)      // timer
  {
    Stop = digitalRead(buttonstop);   // læs status på stop knap
    if (Stop == HIGH) {                // er stop aktiv
      asm volatile ("  jmp 0");       // lav en softstart
    }
    //******************************************************
    // Læs analog input for højre kanal
    //******************************************************
    digitalWrite(LR, HIGH);            // skift til højre kanal
    delay(500); // vent
    valA = (7 * log(((analogRead(analogPin) - 6))) - 30 - OFFSET); // gem højre kanal i valA
    lcd.setCursor(0, 1);               //  position og linje
    lcd.print(" -2 . 1 . 0 . 1 . 2+"); // Tegn skala i LCD
    lcd.setCursor(0, 2);               //  position og linje
    lcd.print("R                   "); // Tegn skala i LCD
    POSR = valA * 5;                   // sæt position
    if (POSR > 7)                      // er differensen større end 7 ( full scale )
    {
      POSR = 8;                        // så lad POS blive 8 ( max udslag )
      lcd.setCursor(0, 2);             //  position og linje
      lcd.print("R                  >"); // og skriv overflow tegn i displayet
    }
    if (POSR < -7)                     // er differensen mindre end 7 ( Minimum scale )
    {
      POSR = -8;                       // så lad POS blive -8 ( min udslag )
      lcd.setCursor(0, 2);             //  position og linje
      lcd.print("R<                  "); // og skriv underflow tegn i displayet
    }
    lcd.setCursor(POSR + 10, 2);         //  sæt position og linje
    lcd.print((char)255);                // Marker grafik

    //******************************************************
    // Læs analog input for venstre kanal
    //******************************************************
    digitalWrite(LR, LOW);            // skift til venstre kanal
    delay(500); // vent
    valB = (7 * log(((analogRead(analogPin) - 6))) - 30 - OFFSET); // gem venstre kanal i valB

    lcd.setCursor(0, 3); //  position og linje
    lcd.print("L                   "); // skala
    POSR = valB * 5;
    if (POSR > 7)
    {
      POSR = 8;
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("L                  >"); // skala
    }
    if (POSR < -7)
    {
      POSR = -8;
      lcd.setCursor(0, 3); //  position og linje
      lcd.print("L<                  "); // skala
    }
    lcd.setCursor(POSR + 10, 3); //  position og linje
    lcd.print((char)255); // Marker
  }
  evc_mute(true);   // mute signalet efter vær delsekvens

}
