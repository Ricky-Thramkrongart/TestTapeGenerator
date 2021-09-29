int del = 100;            //pause interval




void setup() {

  //******************************************************************
  // set I/O mode
  //******************************************************************
  pinMode(0, OUTPUT);    // sets the digital pin 13 as output
  pinMode(1, OUTPUT);    // sets the digital pin 13 as output
  pinMode(2, OUTPUT);    // sets the digital pin 13 as output
  pinMode(3, OUTPUT);    // sets the digital pin 13 as output
  pinMode(4, OUTPUT);    // sets the digital pin 13 as output
  pinMode(5, OUTPUT);    // sets the digital pin 13 as output
  pinMode(6, OUTPUT);    // sets the digital pin 13 as output
  pinMode(7, OUTPUT);    // sets the digital pin 13 as output
  pinMode(8, OUTPUT);    // sets the digital pin 13 as output
  pinMode(9, OUTPUT);    // sets the digital pin 13 as output
  pinMode(10, OUTPUT);    // sets  pin 13 as output Transistor Q4
  pinMode(11, OUTPUT);    // sets  pin 13 as output Transistor Q3
  pinMode(12, OUTPUT);    // sets  pin 13 as output Transistor Q2
  pinMode(13, OUTPUT);    // sets  pin 13 as output Transistor Q1


//*****************************************************************
// sluk alle dioder
//*****************************************************************
  digitalWrite(10, HIGH); // sets Q4 off Channel A LOW  D#
  digitalWrite(11, HIGH); // sets Q3 off Channel A HIGH D#
  digitalWrite(12, HIGH); // sets Q2 off Channel B LOW  D#
  digitalWrite(13, HIGH); // sets Q1 off Channel B HIGH D#



}





void loop() {

  
  // digitalWrite(10, LOW); // sets Q4 on Channel A LOW  D#
  // digitalWrite(11, LOW); // sets Q3 on Channel A HIGH D#
  // digitalWrite(12, LOW); // sets Q2 on Channel B LOW  D#
  // digitalWrite(13, LOW); // sets Q1 on Channel B HIGH D#
//**************************************************************
// diode D2 On/Off    Channel A
//**************************************************************
  digitalWrite(10, LOW); // sets Q4 on Channel A LOW  D#
  digitalWrite(0, HIGH); // sets D1 D2 D3 D4 on
  delay(del);            // waits for a second
  digitalWrite(0, LOW);  // sets D1 D2 D3 D4 OFF
  delay(del);            // waits for a second
  digitalWrite(10, HIGH); // sets Q4 off Channel A LOW  D#
}
