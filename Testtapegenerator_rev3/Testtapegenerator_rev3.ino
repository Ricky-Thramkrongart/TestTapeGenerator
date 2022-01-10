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
int dBset ;
int dBsetarr ;
int CH1 = A0;
int LEFT ;
int CH2 = A1;
int RIGHT ;
int freq = 0;
int   output ;
uint8_t targetChannel0 = 0;
uint8_t targetChannel1 = 0;
uint8_t targetChannel2 = 0;
uint8_t targetChannel3 = 0;
String targetChannelStr = String(targetChannel0);
char stringbuffer[41];

int sekunder;
int minut;
int timer ;
int temp ;
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
  //   dB set
  //----------------------------------------------------------------
  dBset = 3 ;           // setpunkt
  SetdB();  // set output nivau

  //********************************************************
  // Set frekvens
  //----------------------------------------------------------
  freq = 1000;  // set frekvensen
  setFreq(freq);// Og upload til DDS
}





void loop() {
   dBset = 4 ;           // setpunkt
  data();                      // hent data fra Di/Ai og beregn Mavg
  gettime();                   // Hent tid og temp fra RTC
  //**********************************************
  // skriv i display linje 1 og 2
  //----------------------------------------------
  digitalWrite(8, HIGH);             // skift til disp line 1 OG 2
  lcd.setCursor(0, 0); //First line
  sprintf(stringbuffer, "RV0: %03i RV1: %03i RV2: %03i RV2: %03i", set0, set1, set2, set3);  lcd.print(stringbuffer);
  //*******************************************************
  lcd.setCursor(0, 1); //2 line
  sprintf(stringbuffer, "Right: %4i Left: %4i Freq: %4i Hz", RIGHT, LEFT, freq);
  lcd.print(stringbuffer);

sprintf(stringbuffer, "%4i,%4i,%4i", dBset, RIGHT, LEFT );
Serial.println(stringbuffer);




  //*******************************************************************
  // sekund tæller mm.
  //---------------------------------------------------------------------
  int counter = ((minut * 60) + sekunder);
  digitalWrite(8, LOW);        // skift til disp line 3 OG 4
  lcd.setCursor(0, 0);
  sprintf(stringbuffer, "Time: %02i:%02i:%02i  #Avg: %02i  Temp: %2i C", timer, minut, sekunder, awgnum , temp);

  lcd.print(stringbuffer);           // skriv i linje 3

  lcd.setCursor(0, 1);               //4 line
  sprintf(stringbuffer, "Count: %04i Input: %02i  Set: -%2i dBm " , counter, portl, dBset );
  lcd.print(stringbuffer);           // skriv i linje 4

  //-------------------------------------------------------------------------------------------------



}





//*********************************************************************
//   sub rutiner heruinder
//*********************************************************************
//*********************************************************************

void gettime() {
  //-----------------------------------------------------------------------------
  // RTC
  //-------------------------------------------------------------------------------
  //RTC.setDateTime("Jan 24 2021","12:31:00");  set tid og dato
  // ------------------------------------------------------------------------------
  sekunder = (RTC.getSeconds()); // hent og skriv sekunder
  minut = (RTC.getMinutes());    // hent og skriv minutter
  timer = (RTC.getHours());      // hent og skriv timer
  temp = (RTC.getTemp());        // hent rtc temperatur

}



void SetdB() {
  //************************************************************
  //   OUTPUT ARRAY
  //-------------------------------------------------------------
  Serial.println(dBsetarr);
  if (dBset >= 5) {
    digitalWrite(7, HIGH);  // -5 db att ON
    digitalWrite(10, LOW);  // -10 db att OFF
    dBsetarr = dBset - 5 ;  // beregn rest att fra digi-pot
  }
  if (dBset >= 10) {
    digitalWrite(7, LOW);   // -5 db att OFF
    digitalWrite(10, HIGH); // -10 db att ON
    dBsetarr = dBset - 10 ; // beregn rest att fra digi-pot
  }
  if (dBset >= 15) {
    digitalWrite(7, HIGH);  // -5 db att ON
    digitalWrite(10, HIGH); // -10 db att ON
    dBsetarr = dBset - 15 ; // beregn rest att fra digi-pot
  }
  dBsetarr = dBset ; // beregn rest att fra digi-pot
  //************************************************************
  //   OUTPUT ARRAY
  //-------------------------------------------------------------
  int outarr[] { 255, 238, 220, 201, 182, 162, 142, 122, 104, 86, 69, 54, 40, 28, 18, 8, 0 }; // Tabel 0 til 16 giver 0 til - 16dB output


  output = outarr[ dBsetarr ];  //  konverter i array

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
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // input scale
  //--------------
  uint8_t targetChannel2 = 2;
  uint8_t targetValue2 = 45;
  set2 = targetValue2 ; // FLYT DEN OVER I SET2
  uint8_t targetChannel3 = 3;
  uint8_t targetValue3 = 45;
  set3 = targetValue3 ; // FLYT DEN OVER I SET3

  potentio.writeRDAC(targetChannel0, targetValue0);
  potentio.writeRDAC(targetChannel1, targetValue1);
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
  digitalWrite(11, HIGH); // set buzzer on
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
