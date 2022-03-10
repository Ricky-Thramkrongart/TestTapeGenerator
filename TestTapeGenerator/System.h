#pragma once

#include <ArduinoSTL.h>
#include <stack>
#include <EEPROM.h>
#include <I2C_eeprom.h>
#include <fp64lib.h>
#include <SafeString.h>
#include "Relay.h"

#define PROGMEM_FAR  __attribute__((section(".fini7")))

template <class T, std::size_t N>
std::size_t array_size(T(&array)[N]) {
    return N;
}

constexpr int8_t DBIN_MAX = 5;
constexpr int8_t DBIN_MIN_NOPREGAIN = -28 + 5;
constexpr int8_t DBIN_MIN = DBIN_MIN_NOPREGAIN - 12;
constexpr int8_t DBIN_HEADROOM = 2;
constexpr int8_t DBIN_MAX_SERVICE = DBIN_MAX - DBIN_HEADROOM;
constexpr int8_t DBIN_MIN_SERVICE = DBIN_MIN + DBIN_HEADROOM;

constexpr int8_t DBOUT_MAX = 0;
constexpr int8_t DBOUT_MIN = -16 - 10 - 20;
constexpr int8_t DBOUT_HEADROOM = 5;
constexpr int8_t DBOUT_MAX_SERVICE = DBOUT_MAX - DBOUT_HEADROOM;
constexpr int8_t DBOUT_MIN_SERVICE = DBOUT_MIN + DBOUT_HEADROOM;

constexpr double MAX_DEVICE_STD_DEV = 0.1;

bool Is_dBIn_OutOfRange(const double dB)
{
    if (dB > DBIN_MAX || dB < DBIN_MIN) {
        Serial.print(F("DBIN_MIN: ")); Serial.println(DBIN_MIN);
        Serial.print(F("DBIN_MAX: ")); Serial.println(DBIN_MAX);
        Serial.print(F("DBIN_MIN_NOPREGAIN: ")); Serial.println(DBIN_MIN_NOPREGAIN);
        Serial.print(F("Is_dBIn_OutOfRange: ")); Serial.println(dB);
        delay(1000);
        return true;
    }
    return false;
}

bool Is_dBIn_OutOfRange(const std::pair<double, double>& dB)
{
    if (Is_dBIn_OutOfRange(dB.first) || Is_dBIn_OutOfRange(dB.second)) {
        return true;
    }
    return false;
}

bool Is_dBOut_OutOfRange(const double dB)
{
    if (dB > DBOUT_MAX || dB < DBOUT_MIN) {
        Serial.print(F("DBOUT_MIN: ")); Serial.println(DBOUT_MIN);
        Serial.print(F("DBOUT_MAX: ")); Serial.println(DBOUT_MAX);
        Serial.print(F("Is_dBOut_OutOfRange: ")); Serial.println(dB);
        delay(1000);
        return true;
    }
    return false;
}

bool Is_dBOut_OutOfRange(const std::pair<double, double>& dB)
{
    if (Is_dBOut_OutOfRange(dB.first) || Is_dBOut_OutOfRange(dB.second)) {
        return true;
    }
    return false;
}

char* uintToStr(unsigned long x, unsigned d, char* b)
{
    char* p;
    unsigned digits = 0;
    unsigned long t = x;

    do ++digits; while (t /= 10);
    // if (digits > d) d = digits; // uncomment to allow more digits than spec'd
    *(p = b + d) = '\0';
    do *--p = x % 10 + '0'; while (x /= 10);
    while (p != b) *--p = ' ';
    return b;
}

char* uintToStr(const uint64_t num, char* str)
{
    uint8_t i = 0;
    uint64_t n = num;

    do
        i++;
    while (n /= 10);

    str[i] = '\0';
    n = num;

    do
        str[--i] = (n % 10) + '0';
    while (n /= 10);

    return str;
}
class SignalGenerator;
class dBMeter;

class System
{
private:
    //Disables copy constructor
    System(const System&) = delete;
    System(System&) = delete;
    //Disables assignment operator
    System& operator=(const System&) = delete;
    System& operator=(System&) = delete;

protected:
    static Relay muteRelay;
    static Relay calibrationRelay;
    static std::stack<std::pair<bool, bool>> relayStack;

public:
    static std::size_t fit64_size;
    static std::size_t fit64RV45_l_size;
    static std::size_t fit64RV45_r_size;
    static const float64_t* fit64;
    static const float64_t* fit64RV45_l;
    static const float64_t* fit64RV45_r;
    static std::pair<double, double>System::_5dBInputAttenuator;

    static void PopRelayStack()
    {
        if (!relayStack.empty()) {
            std::pair<bool, bool> relayStatus = relayStack.top();
            relayStack.pop();
            if (relayStatus.first)
                muteRelay.Enable();
            else
                muteRelay.Disable();
            if (relayStatus.second)
                calibrationRelay.Enable();
            else
                calibrationRelay.Disable();
        }
    }

    static void PrintRelayState()
    {
        //Serial.print(F("M: ")); Serial.print(System::GetMute());
        //Serial.print(F(" C: ")); Serial.print(System::GetCalibration());
        //Serial.print(F(" Stack: ")); Serial.println(relayStack.size());
    }

    static void OutPutOn()
    {
        relayStack.push({ muteRelay.IsEnabled(), calibrationRelay.IsEnabled() });
        muteRelay.Disable();
        calibrationRelay.Disable();
        //Serial.print(F("UnMute(): "));
        PrintRelayState();
    }

    static void OutPutOff()
    {
        relayStack.push({ muteRelay.IsEnabled(), calibrationRelay.IsEnabled() });
        muteRelay.Enable();
        calibrationRelay.Disable();
        //Serial.print(F("OutPutOff(): "));
        PrintRelayState();
    }

    static void InternalMeasurementOn()
    {
        relayStack.push({ muteRelay.IsEnabled(), calibrationRelay.IsEnabled() });
        muteRelay.Enable();
        calibrationRelay.Enable();
        //Serial.print(F("InternalMeasurementOn(): "));
        PrintRelayState();
    }

    static bool GetCalibration()
    {
        return  calibrationRelay.IsEnabled();
    }
    static bool GetMute()
    {
        return  muteRelay.IsEnabled();
    }

    //static void WriteFit64ToEEPROM(void)
    //{
    //    int addrOffset = 0;
    //    //byte size_ = fit64.size();
    //    byte size_ = System::fit64_size();
    //    EEPROM.put(addrOffset, size_);
    //    addrOffset += sizeof(size_);
    //    for (int i = 0; i != size_; i++)
    //    {
    //        EEPROM.put(addrOffset, fit64[i]);
    //        addrOffset += sizeof(float64_t);
    //    }
    //}

    //static void ReadFit64FromEEPROM(void)
    //{
    //    int addrOffset = 0;
    //    byte size_;
    //    EEPROM.get(addrOffset, size_);
    //    addrOffset += sizeof(size_);
    //    fit64.resize(size_);
    //    for (int i = 0; i != size_; i++)
    //    {
    //        EEPROM.get(addrOffset, fit64[i]);
    //        addrOffset += sizeof(float64_t);
    //    }
    //}

    //static void WriteFit64ToI2C_eeprom(uint16_t& addr_, std::vector<float64_t>& fit64) {
    //    I2C_eeprom i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512);
    //    i2C_eeprom.writeByte(addr_, fit64.size());
    //    uint8_t arraysize = i2C_eeprom.readByte(addr_);
    //    if (arraysize != fit64.size()) {
    //        Serial.println("I2C_eeprom Error");
    //    }
    //    addr_ += 1;
    //    uint8_t datasize = fit64.size() * sizeof(float64_t);
    //    i2C_eeprom.writeBlock(addr_, (uint8_t*)fit64.begin(), datasize);
    //    addr_ += datasize;
    //    Serial.print(F("fit64 written. addr: "));
    //    Serial.println(addr_);
    //}

    //static bool ReadFit64FromI2C_eeprom(uint16_t& addr_, std::vector<float64_t>& fit64) {
    //    I2C_eeprom i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512);
    //    uint8_t arraysize = i2C_eeprom.readByte(addr_);
    //    Serial.print(F("Read Arraysize: "));
    //    Serial.println(arraysize);
    //    if (arraysize == 0 || arraysize == 0xFF || arraysize > 20) {
    //        return false;
    //    }
    //    addr_ += 1;
    //    fit64 = std::vector<float64_t>(arraysize);
    //    uint8_t datasize = fit64.size() * sizeof(float64_t);
    //    i2C_eeprom.readBlock(addr_, (uint8_t*)fit64.begin(), datasize);
    //    addr_ += datasize;
    //    Serial.print(F("fit64 read. addr: "));
    //    Serial.println(addr_);
    //    return true;
    //}

    //static void WriteFit64ToI2C_eeprom()
    //{
    //    I2C_eeprom i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512);
    //    i2C_eeprom.begin();
    //    if (i2C_eeprom.isConnected()) {
    //        uint16_t addr(0);
    //        std::vector<std::vector<float64_t>*> fit64s = { &fit64RV45_l, &fit64RV45_r };
    //        for (std::vector<std::vector<float64_t>*>::iterator i = fit64s.begin(); fit64s.end() != i; i++) {
    //            if (!(*i)->empty()) {
    //                WriteFit64ToI2C_eeprom(addr, **i);
    //            }
    //        }

    //    }
    //}

    //static bool ReadFit64FromI2C_eeprom(void)
    //{
    //    I2C_eeprom i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512);
    //    i2C_eeprom.begin();
    //    if (i2C_eeprom.isConnected()) {
    //        uint16_t addr(0);
    //        std::vector<std::vector<float64_t>*> fit64s = { &fit64RV45_l, &fit64RV45_r };
    //        for (std::vector<std::vector<float64_t>*>::iterator i = fit64s.begin(); fit64s.end() != i; i++) {
    //            bool result = ReadFit64FromI2C_eeprom(addr, **i);
    //            if (!result) {
    //                Serial.println("ReadFit64FromI2C_eeprom Error");
    //                return false;
    //            }
    //        }
    //        return true;
    //    }
    //    return false;
    //}
    static void begin(void) {
    }

    static void DumpData(Stream& out, const float64_t* f, std::size_t f_size, const String& s) {
        for (int i = 0; i != f_size; i++) {
            char buffer[21];
            cSF(sf_line, 128);
            sf_line.print(F("        ")); sf_line.print(s.c_str()); sf_line.print(F("[")); sf_line.print(f_size - i - 1);  sf_line.print(F("] = ")); sf_line.print(uintToStr(f[f_size - i - 1], buffer)); sf_line.print(F("LL; //")); sf_line.print(fp64_to_string(f[f_size - i - 1], 32, 2));            out.println(sf_line.c_str());
        }
    }

    static void DumpData(void) {
        DumpData(Serial, fit64, fit64_size, String(F("fit64")));
        DumpData(Serial, fit64RV45_l, fit64RV45_l_size, String(F("fit64RV45_l")));
        DumpData(Serial, fit64RV45_r, fit64RV45_r_size, String(F("fit64RV45_r")));
    }

    static void System::SetupDevice();
    static void Device1(void) {
        static const float64_t fit64_1[] PROGMEM_FAR = {
            4643170361447727206LL, //254.83884893125213
            4625311246433173744LL, //16.406533912931025
            13826466162739603870LL, //-0.54610830741448857
            13804946084766212980LL, //-0.020528115676117634
            13794443971273051489LL, //-3.8708237535246969E-3
            13787028885511535108LL, //-1.3040839584018702E-3
            13771625268806377890LL, //-1.1688793870179859E-4
            13750155056991901613LL, //-4.3783084399724716E-6
            13722620578406386687LL //-6.1623148223802267E-8
        };
        static const float64_t fit64RV45_l_1[] PROGMEM_FAR = {
            13852328171936497664LL, //-29.355779672332574
            4598729032677225461LL, //0.28074280452147998
            13794669453749475540LL, //-4.0309723796864878E-3
            4542466373570639892LL, //4.9748269797742075E-5
            13735106658844864340LL, //-4.3066302928320864E-7
            4478279631990624686LL, //2.5664114005358071E-9
            13666029766803593906LL, //-1.0682418823283996E-11
            4405021524398216095LL, //3.1584024533829449E-14
            13588296773283881548LL, //-6.7056839408840926E-17
            4322959140154988601LL, //1.0244379036302585E-19
            13502034192772973347LL, //-1.115804991376365E-22
            4231737999454206243LL, //8.4506437891524204E-26
            13405751706773511980LL, //-4.227894224574707E-29
            4129886612513857885LL, //1.2560653304131632E-32
            13295144786331457814LL, //-1.6779235851105009E-36
        };
        static const float64_t fit64RV45_r_1[] PROGMEM_FAR = {
            13852251674767900672LL, //-29.084007135068532
            4598738819454652931LL, //0.28128607980332293
            13794839316444044651LL, //-4.1783047816673453E-3
            4542962144645554073LL, //5.3107745275726005E-5
            13735858084951937328LL, //-4.7044319616557479E-7
            4478984521471939029LL, //2.8579468570036992E-9
            13666909680314710262LL, //-1.2103995770781873E-11
            4405780903599719474LL, //3.6376381048261166E-14
            13589221222723261109LL, //-7.8451558496028697E-17
            4324006261809167393LL, //1.2167590034249164E-19
            13503007680088403932LL, //-1.3446707593535264E-22
            4233372120118173711LL, //1.0326522313620901E-25
            13407382428027780173LL, //-5.2351804072132252E-29
            4131051555248098586LL, //1.5748997961393054E-32
            13296494190582714881LL //-2.1287535929115348E-36
        };
        _5dBInputAttenuator = { -5.21, -5.20 };

        fit64 = fit64_1;
        fit64RV45_l = fit64RV45_l_1;
        fit64RV45_r = fit64RV45_r_1;
        fit64_size = array_size(fit64_1);
        fit64RV45_l_size = array_size(fit64RV45_l_1);
        fit64RV45_r_size = array_size(fit64RV45_r_1);
        //DumpData();
    }

    static void Device2(void) {
        static const float64_t fit64_2[] PROGMEM_FAR = {
            4643186195694485085LL, //255.28888529165388        
            4625656428956659744LL, //17.632868585802612
            4587701044841159231LL, //0.052320812997141967
            4595035821655076603LL, //0.16286420447101321
            4583100558110554195LL, //0.025824261833570294
            4564020665793110761LL, //1.382974224325812E-3
            4538297697832495092LL //2.6008901174857894E-5
        };
        static const float64_t fit64RV45_l_2[] PROGMEM_FAR = {
            13858875308489361013LL, //-82.463525042659811
            4612427963337731302LL, //2.3294897289866752
            13809812090832753964LL, //-0.043570931183732092
            4557854103742579067LL, //5.1118836496759302E-4
            13749692381677985710LL, //-3.9864072052044757E-6
            4492116402934755415LL, //2.1609842284031454E-8
            13679411735363808524LL, //-8.3794257702177334E-11
            4418208780947297804LL, //2.3633748281711946E-13
            13601319233535339934LL, //-4.8830080849180298E-16
            4335617576435301208LL, //7.3747078999019286E-19
            13514842005795431764LL, //-8.037299976392854E-22
            4244580521380718716LL, //6.1467995734380623E-25
            13418692839575060986LL, //-3.1268682744213197E-28
            4142976882641519474LL, //9.494262059115952E-32
            13308501763921255139LL //-1.3012260559125937E-35
        };
        static const float64_t fit64RV45_r_2[] PROGMEM_FAR = {
            13858859640482057285LL, //-82.240869267189808
            4612466344284899237LL, //2.3465342934877804
            13809933705389921679LL, //-0.044414801693089527
            4558000318475425675LL, //5.2704099806776103E-4
            13749889561846793348LL, //-4.1534253047297098E-6
            4492455254634098528LL, //2.2731008508940537E-8
            13679806778883609627LL, //-8.8900075525523905E-11
            4418532211350909990LL, //2.5266654527916786E-13
            13601697810825682513LL, //-5.2563141148627357E-16
            4336254097730132913LL, //7.9876556108801871E-19
            13515453406875884884LL, //-8.7541881060975614E-22
            4245214973053452452LL, //6.7294514149757634E-25
            13419390032310336391LL, //-3.4395003093908697E-28
            4143598898127536967LL, //1.0489683655067018E-31
            13309034534757754102LL //-1.4436231649486664E-35
        };
        _5dBInputAttenuator = { -5.11,-5.03 };

        fit64 = fit64_2;
        fit64RV45_l = fit64RV45_l_2;
        fit64RV45_r = fit64RV45_r_2;
        fit64_size = array_size(fit64_2);
        fit64RV45_l_size = array_size(fit64RV45_l_2);
        fit64RV45_r_size = array_size(fit64RV45_r_2);
        //DumpData();
    }

    static uint16_t OutPutFit64(const double dB)
    {
        float64_t dB64 = fp64_sd(dB);
        float64_t dB64_pow = fp64_sd(1.0);

        float64_t rv = fit64[0];
        for (int i = 1; i != fit64_size; ++i)
        {
            dB64_pow = fp64_mul(dB64, dB64_pow);
            rv = fp64_add(rv, fp64_mul(fit64[i], dB64_pow));
        }

        //rv = fp64_round(rv);
        rv = fp64_fmin(rv, fp64_sd(255));
        rv = fp64_fmax(rv, fp64_sd(0));
        return fp64_to_uint16(rv);
    }

    static double PolyVal(const float64_t fit64[], const std::size_t fit64_size, uint16_t v, double offset)
    {
        float64_t x = fp64_sd(v);
        float64_t x_pow = fp64_sd(1.0);
        float64_t y = fit64[0];
        for (int i = 1; i != fit64_size; ++i)
        {
            x_pow = fp64_mul(x, x_pow);
            y = fp64_add(y, fp64_mul(fit64[i], x_pow));
        }

        y = fp64_fmax(y, fp64_sd(-29.2));
        y = fp64_fmin(y, fp64_sd(0));

        return fp64_ds(y) + offset;
    }
};
const float64_t* System::fit64;
const float64_t* System::fit64RV45_l;
const float64_t* System::fit64RV45_r;

std::size_t System::fit64_size;
std::size_t System::fit64RV45_l_size;
std::size_t System::fit64RV45_r_size;

std::stack<std::pair<bool, bool>> System::relayStack;
std::pair<double, double>System::_5dBInputAttenuator({ 0.0, 0.0 });
Relay System::muteRelay(Relay(28, true));
Relay System::calibrationRelay(Relay(26));


