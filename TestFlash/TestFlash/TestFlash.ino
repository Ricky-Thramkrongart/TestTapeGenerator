/*
 Name:		TestFlash.ino
 Created:	3/3/2022 7:17:11 PM
 Author:	Admin
*/

// the setup function runs once when you press reset or power the board
const char Flash0[] = "123456789ABCDEF0";
const uint64_t Flash32K_1[INT16_MAX / 8] PROGMEM = { 0LL };
const uint64_t Flash32K_2[INT16_MAX / 8] PROGMEM = { 0LL };
const uint64_t Flash32K_3[INT16_MAX / 8] PROGMEM = { 0LL };
const uint64_t Flash32K_4[INT16_MAX / 8] PROGMEM = { 0LL };
const uint64_t Flash32K_5[INT16_MAX / 8] PROGMEM = { 0LL };
const uint64_t Flash32K_6[INT16_MAX / 8] PROGMEM = { 0LL };
const uint64_t Flash32K_7[INT16_MAX / 8] PROGMEM = { 0LL };
const uint64_t Flash32K_8[800] PROGMEM = { 0LL };
const char Flash9[] = "123456789ABCDEF0";

uint64_t f(const uint64_t* a, size_t s)
{
    uint64_t r = 0;
    for (int i = 0; i != s; ++i)
    {
        r += a[i];
    }
    return r;
}

template <class T, size_t N>
size_t array_size(T(&array)[N]) {
    return N;
}

void setup() {
    Serial.begin(115200);
    Serial.println(Flash0);
    Serial.println(Flash9);
    delay(1000);
    int i =
        f(Flash32K_1, array_size(Flash32K_1)) +
        f(Flash32K_2, array_size(Flash32K_2)) +
        f(Flash32K_3, array_size(Flash32K_3)) +
        f(Flash32K_4, array_size(Flash32K_4)) +
        f(Flash32K_5, array_size(Flash32K_5)) +
        f(Flash32K_6, array_size(Flash32K_6)) +
        f(Flash32K_7, array_size(Flash32K_7)) +
        f(Flash32K_8, array_size(Flash32K_8));
    Serial.println(i);
    Serial.println(Flash0);
    Serial.println(Flash9);
    delay(1000);
}

// the loop function runs over and over again until power down or reset
void loop() {
}
