#pragma once

void Beep(long ms = 30)
{
    digitalWrite(11, HIGH); // set buzzer on
    delay(ms);
    digitalWrite(11, LOW); // set buzzer OFF
}
