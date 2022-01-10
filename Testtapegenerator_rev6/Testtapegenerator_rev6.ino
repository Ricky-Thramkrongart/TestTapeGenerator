// *********************************************************************
//  Test software for Analyzer
// *********************************************************************

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x25, 40, 2);
#include <Wire.h>
#include <RTC.h>
static DS3231 RTC;
//-------------------------------------------------------------------------
#include <movingAvg.h>                       // include bib for Mavg
int awgnum = 5;                              // start antal af mawg
movingAvg leftavg(awgnum);                   // define the moving average object
movingAvg rightavg(awgnum);                  // define the moving average object
//**********************************************************************
// set af addr og start af bib for digital potmeter
//**********************************************************************
#include <AD5254_asukiaaa.h>
AD5254_asukiaaa potentio(AD5254_ASUKIAAA_ADDR_A0_GND_A1_GND);
// adressen er 0x2C da A0_GND og A1_GND

// *********************************************************************
// setup af udgange til DDS
//**********************************************************************
const uint8_t _dataPin  = 9;
const uint8_t _clkPin   = 13;
const uint8_t _fsyncPin = 2;

//**********************************************************************
// variabel
//**********************************************************************
byte portl;
int set0 ;
int set1  ;
int set2  ;
int set3  ;
int dBset_out ;
int dBsetarr ;
int CH1 = A0;
int CH2 = A1;
int LEFT ;
float LEFTMAT ;
int RIGHT ;
float RIGHTMAT ;
int freq = 0;
int   output ;
int sekunder;
int minut;
int timer ;
int temp ;
uint8_t targetChannel0 = 0;
uint8_t targetChannel1 = 0;
uint8_t targetChannel2 = 0;
uint8_t targetChannel3 = 0;
String targetChannelStr = String(targetChannel0);
char stringbuffer[41];
unsigned long counter;
unsigned long countnu;
int nsl;
int Rightscaled;
int Leftscaled;
int  input_scale;
//*****************************************************************
// DDS generator
//-----------------------------------------------------------------
// send raw 16-bit word
void spiSend(const uint16_t data)
{
  digitalWrite(_fsyncPin, LOW);

  uint16_t m = 1UL << 15;
  for (uint8_t i = 0; i < 16; i++)
  {
    digitalWrite(_dataPin, data & m ? HIGH : LOW);
    digitalWrite(_clkPin, LOW); //data is valid on falling edge
    digitalWrite(_clkPin, HIGH);
    m >>= 1;
  }
  digitalWrite(_dataPin, LOW); //idle low
  digitalWrite(_fsyncPin, HIGH);
}

void setFreq(double f)
{
  const uint16_t b28  = (1UL << 13);
  const uint16_t freq = (1UL << 14);

  const double   f_clk = 25e6;
  const double   scale = 1UL << 28;
  const uint32_t n_reg = f * scale / f_clk;

  const uint16_t f_low  = n_reg         & 0x3fffUL;
  const uint16_t f_high = (n_reg >> 14) & 0x3fffUL;

  spiSend(b28);
  spiSend(f_low  | freq);
  spiSend(f_high | freq);
}


//*********************************************************************
//**************************************************************************
// SETUP
//**************************************************************************
void setup() {

  //-------------------------------------------------------
  //  setup af DI/DO
  //-------------------------------------------------------
  pinMode(26, OUTPUT); // internal calib relæ
  pinMode(27, OUTPUT); // aux relæ 2
  pinMode(28, OUTPUT); // output on relæ
  pinMode(29, OUTPUT); // aux relæ 1
  pinMode(7, OUTPUT);  // output att fixed -5dB
  pinMode(8, OUTPUT);  // disp line select
  pinMode(10, OUTPUT); // output att fixed -10dB
  pinMode(30, OUTPUT); // input att fixed xxxdB ( kun ny hardware )
  pinMode(28, OUTPUT); // output on relæ
  pinMode(11, OUTPUT); // Buzzer
  DDRL = B00000000; // all inputs PORT-L D42 til D49
  pinMode(_clkPin,   OUTPUT);
  pinMode(_fsyncPin, OUTPUT);
  pinMode(_dataPin,  OUTPUT);
  //---------------------------------------------------------
  digitalWrite(28, HIGH);  // set output relæ on
  digitalWrite(26, HIGH);  // cal relæ relæ on
  digitalWrite(11, LOW);   // set buzzer on
  digitalWrite(8, HIGH);   // disp line
  digitalWrite(27, LOW);   // aux relæ 2 on/off
  digitalWrite(29, LOW);   // aux relæ 1 on/off
  digitalWrite(11, LOW);   // set buzzer OFF
  digitalWrite(7, LOW);    // output att fixed -5dB
  digitalWrite(10, LOW);   // output att fixed -10dB
  digitalWrite(30, LOW);   // input att fixed
  digitalWrite(_fsyncPin, HIGH);
  digitalWrite(_clkPin,   LOW);
  //-----------------------------------------------------------
  Serial.begin(115200);    // start serial com på 115200 Baud
  potentio.begin();        // start Didital potmeter

  RTC.begin();             // start RTC
  RTC.setHourMode(CLOCK_H24); // set til 24 timer
  //------------------------------------------------------------
  leftavg.begin();        // Moving avg start left
  rightavg.begin();       // Moving avg start right
  //------------------------------------------------------------

  lcd.init();              // initialize the LCD
  lcd.setBacklight(HIGH);  // SET LCD LYS ON / OFF
  lcd.clear();             // clear Display

  splashscreen();   // KALD SPLACHSCREEN

  //*****************************************************************
  //   dB set på output
  //----------------------------------------------------------------
  //dBset_out = 0;           // setpunkt
  //SetdBout();             // kald set output nivau rutine

  //*****************************************************************
  //   dB set på input
  //----------------------------------------------------------------
  input_scale=45;
  SetdBin();    // set scaler på input

  //********************************************************
  // Set frekvens
  //----------------------------------------------------------
  freq = 100;  // set frekvensen
  setFreq(freq);// Og upload til DDS
}



int i = 0;
//*********************************************************************
//
//*********************************************************************
void loop() {
  data();                        // hent måle data 
  gettime();                     // hent tiden
  counter = millis()/1000;       // sekund tæller

  if (counter >= countnu + 3) {
    countnu = counter;
    dBset_out = i;               // setpunkt for output
    SetdBout();                  //  kald set output nivau rutine

    i = i + 1;
    if (i >= 15) {
      i = 0;

    }
  }

  //**********************************************
  // skriv i display linje 1 og 2
  //----------------------------------------------
  digitalWrite(8, HIGH);             // skift til disp line 1 OG 2
  lcd.setCursor(0, 0);               //First line
  sprintf(stringbuffer, "RV0: %03i RV1: %03i RV2: %03i RV2: %03i", set0, set1, set2, set3);  lcd.print(stringbuffer);
  //*******************************************************

  RIGHTMAT = (71.928 - (31.3812 * pow(RIGHT, 0.119992))); // skalering af input til dB 15 max
  LEFTMAT = (71.928 - (31.3812 * pow(LEFT, 0.119992))); // skalering af input til dB 15 max
  // RIGHTMAT = (46.3906 - (11.5751 * pow(RIGHT, 0.201542))); // skalering af input til dB 25 max
Rightscaled = RIGHTMAT * 10 ;     // skaleret outpot i dB gange 10 (right)
Leftscaled = LEFTMAT * 10 ;       // skaleret outpot i dB gange 10 ( Left)

  sprintf(stringbuffer, "Right: %4i Left: %4i Freq: %4i Hz", RIGHT, LEFT, freq);
  lcd.setCursor(0, 1); //2 line
  lcd.print(stringbuffer);

  //sprintf(stringbuffer, "%4i,%4i,%4i", dBset_out, RIGHT, LEFT );
  // Serial.println(RIGHTMAT);


  //*******************************************************************
  // Skriv i display linje 3 og 4        sekund tæller mm.
  //---------------------------------------------------------------------

  digitalWrite(8, LOW);        // skift til disp line 3 OG 4
  lcd.setCursor(0, 0);
  sprintf(stringbuffer, "Time: %02i:%02i:%02i  #Avg: %02i  Temp: %2i C", timer, minut, sekunder, awgnum , temp);

  lcd.print(stringbuffer);           // skriv i linje 3

  lcd.setCursor(0, 1);               //4 line
  sprintf(stringbuffer, "L_R:%2i.%1i dB L_L:%2i.%1i dB  Set: -%2i dBm " , Rightscaled / 10, (Rightscaled % 10), Leftscaled / 10, (Leftscaled % 10), dBset_out );
  lcd.print(stringbuffer);           // skriv i linje 4

  //-------------------------------------------------------------------------------------------------


}





//*********************************************************************
//   sub rutiner heruinder
//*********************************************************************
//*********************************************************************

void gettime() {
  //-----------------------------------------------------------------------------
  // RTC      // Hent tid og temp fra RTC
  //-------------------------------------------------------------------------------
  //RTC.setDateTime("Jan 24 2021","12:31:00");  set tid og dato
  // ------------------------------------------------------------------------------
  sekunder = (RTC.getSeconds()); // hent og skriv sekunder
  minut = (RTC.getMinutes());    // hent og skriv minutter
  timer = (RTC.getHours());      // hent og skriv timer
  temp = (RTC.getTemp());        // hent rtc temperatur

}



void SetdBout() {
  //************************************************************
  //   OUTPUT ARRAY
  //-------------------------------------------------------------

  digitalWrite(7, LOW);  // -5 db att OFF
  digitalWrite(10, LOW);  // -10 db att OFF

  dBsetarr = dBset_out ; // beregn rest att fra digi-pot
  if (dBset_out >= 5) {
    digitalWrite(7, HIGH);  // -5 db att ON
    digitalWrite(10, LOW);  // -10 db att OFF
    dBsetarr = dBset_out - 5 ;  // beregn rest att fra digi-pot
  }
  if (dBset_out >= 10) {
    digitalWrite(7, LOW);   // -5 db att OFF
    digitalWrite(10, HIGH); // -10 db att ON
    dBsetarr = dBset_out - 10 ; // beregn rest att fra digi-pot
  }
  if (dBset_out >= 15) {
    digitalWrite(7, HIGH);  // -5 db att ON
    digitalWrite(10, HIGH); // -10 db att ON
    dBsetarr = dBset_out - 15 ; // beregn rest att fra digi-pot
  }

  //************************************************************
  //   OUTPUT ARRAY
  //-------------------------------------------------------------

  int outarr[] {  255, 238, 220, 201, 182, 162, 142, 122, 104, 86, 69, 54, 40, 28, 18, 8, 0 }; // Tabel 0 til 16 giver 0 til - 16dB output

  output = outarr[ (dBsetarr) ];  //  konverter i array

  //************************************************************
  // Set attnuator nivauer i Digi-Pot
  //************************************************************
  // output scale
  //---------------
  uint8_t targetChannel0 = 0;
  uint8_t targetValue0 = output;
  set0 = targetValue0 ; // FLYT DEN OVER I SET0
  uint8_t targetChannel1 = 1;
  uint8_t targetValue1 = output;
  set1 = targetValue1 ; // FLYT DEN OVER I SET1
  potentio.writeRDAC(targetChannel0, targetValue0);  // skriv til Digi pot
  potentio.writeRDAC(targetChannel1, targetValue1);

  return ;
}

void SetdBin() {
  //************************************************************
  //  input skalering
  //-------------------------------------------------------------
  uint8_t targetChannel2 = 2;
  uint8_t targetValue2 = input_scale;
  set2 = targetValue2 ; // FLYT DEN OVER I SET2
  uint8_t targetChannel3 = 3;
  uint8_t targetValue3 = input_scale;
  set3 = targetValue3 ; // FLYT DEN OVER I SET3


  potentio.writeRDAC(targetChannel2, targetValue2);
  potentio.writeRDAC(targetChannel3, targetValue3);

  return ;
}

void data() {
  //=============================================================
  // LÆS RÅ DATA Ai/Di  OG BEREGN MOVING Awg
  //-------------------------------------------------------------
  // frekvens afhængig moving AVG herunder
  //-------------------------------------------------------------
  if (freq < 300) {
    awgnum = map((freq), 0, 300, 50, 5);//er freq < 300 Hz Mavg = dynamisk
  }
  if (freq >= 300) {
    awgnum = 5;                         // er freq > 500 hz Mavg = 5 punkter
  }

  int LEFTRAW = analogRead(CH1);        // læs rådata fra kanal 1
  int RIGHTRAW = analogRead(CH2);       // læs rådata fra kanal 2

  LEFT = leftavg.reading(LEFTRAW);      // calculate the moving average
  RIGHT = rightavg.reading(RIGHTRAW);   // calculate the moving average
 
  portl = PINL;                         // læs hele port L
  return ;
}
//--------------------------------------------------------------------------


void splashscreen() {
  //========================================================================
  //------BEEP--------------------------------------
  digitalWrite(11, LOW); // set buzzer on
  delay(30);
  digitalWrite(11, LOW); // set buzzer OFF
  //------------------------------------------------
  digitalWrite(8, LOW);        // skift til disp line 3 OG 4
  lcd.clear();             // clear Display
  lcd.init();
  digitalWrite(8, HIGH);        // skift til disp line 3 OG 4
  lcd.clear();
  lcd.init();
  lcd.setCursor(0, 0);         //1 line
  lcd.print("         Test Tape Generator  ");
  lcd.setCursor(0, 1);         //2 line
  lcd.print("     Software ver: 2021-07-27-NSL  ");
  delay(500);
  lcd.clear();
  digitalWrite(8, LOW);       //skift til disp line 1 OG 2
  lcd.clear();
  lcd.init();
  return;
}
//**************************************************************************
