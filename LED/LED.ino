// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       LED.ino
    Created:	03-09-2020 11:37:54
    Author:     MD-MAN\rct
*/


class LED 
{
public:

    enum Channel { Chn_A = 0, Chn_B = 1 };
    Channel chn;

    LED(Channel chn_):chn(chn_)
    {
        for (int i = 0; i < 10; ++i) {
            pinMode(i, INPUT);    // sets  pin 13 as output Transistor Q4
        }
        switch (this->chn)
        {
        case Chn_A:
            pinMode(10, OUTPUT);    // sets  pin 13 as output Transistor Q4
            digitalWrite(10, HIGH); // sets Q4 off Channel A LOW  D#
            pinMode(11, OUTPUT);    // sets  pin 13 as output Transistor Q3
            digitalWrite(11, HIGH); // sets Q3 off Channel A HIGH D#
            break;

        case Chn_B:
            pinMode(12, OUTPUT);    // sets  pin 13 as output Transistor Q2
            digitalWrite(12, HIGH); // sets Q2 off Channel B LOW  D#
            pinMode(13, OUTPUT);    // sets  pin 13 as output Transistor Q1
            digitalWrite(13, HIGH); // sets Q1 off Channel B HIGH D#
            break;
        }
    }

    void SetValue(int v) 
    {
        for (int i = 0; i < 10; ++i) {
            pinMode(i, INPUT);    // sets  pin 13 as output Transistor Q4
        }

        v /= 10;
        int x = 0; //v/2;
        switch (this->chn)
        {
        case Chn_A:
            if (x) {
                digitalWrite(10, HIGH); // sets Q4 on Channel A LOW  D#
                pinMode(v, INPUT);
            }
            digitalWrite(10+x, LOW); // sets Q4 on Channel A LOW  D#
            pinMode(v, OUTPUT);
            digitalWrite(v, LOW);
            break;

        case Chn_B:
            digitalWrite(10, LOW); // sets Q4 on Channel A LOW  D#
            pinMode(v, OUTPUT);
            digitalWrite(v, LOW);
            break;
        }

    }
};






LED ledA(LED::Channel::Chn_A);
LED ledB(LED::Channel::Chn_B);
// The setup() function runs once each time the micro-controller starts
void setup()
{
}

// Add the main program code into the continuous loop() function
void loop()
{
    for (int i = 0; i < 100; ++i) {
        ledA.SetValue(i);
        delay(100);
    }
}
