#pragma once

#include <ArduinoSTL.h>
#include <EEPROM.h>
#include <I2C_eeprom.h>
#include "Relay.h"
#include <fp64lib.h>

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
public:
    static void UnMute()
    {
        muteRelay.Disable();
        calibrationRelay.Disable();
    }

    static void Mute()
    {
        muteRelay.Enable();
        calibrationRelay.Disable();
    }

    static void CalibrationMode()
    {
        muteRelay.Enable();
        calibrationRelay.Enable();
    }

    static void UnmutedCalibrationMode()
    {
        muteRelay.Disable();
        calibrationRelay.Enable();
    }

    static bool GetCalibration()
    {
        return  calibrationRelay.IsEnabled();
    }
    static bool GetMute()
    {
        return  muteRelay.IsEnabled();
    }

    static void WriteFit64ToEEPROM(void)
    {
        int addrOffset = 0;
        byte size_ = fit64.size();
        EEPROM.put(addrOffset, size_);
        addrOffset += sizeof(size_);
        for (int i = 0; i != size_; i++)
        {
            EEPROM.put(addrOffset, fit64[i]);
            addrOffset += sizeof(float64_t);
        }
    }

    static void ReadFit64FromEEPROM(void)
    {
        int addrOffset = 0;
        byte size_;
        EEPROM.get(addrOffset, size_);
        addrOffset += sizeof(size_);
        fit64.resize(size_);
        for (int i = 0; i != size_; i++)
        {
            EEPROM.get(addrOffset, fit64[i]);
            addrOffset += sizeof(float64_t);
        }
    }

    static void WriteFit64ToI2C_eeprom(uint16_t& addr_, std::vector<float64_t>& fit64) {
        I2C_eeprom i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512);
        i2C_eeprom.writeByte(addr_, fit64.size());
        uint8_t arraysize = i2C_eeprom.readByte(addr_);
        if (arraysize != fit64.size()) {
            Serial.println("I2C_eeprom Error");
        }
        addr_ += 1;
        uint8_t datasize = fit64.size() * sizeof(float64_t);
        i2C_eeprom.writeBlock(addr_, (uint8_t*)fit64.begin(), datasize);
        addr_ += datasize;
        Serial.print(F("fit64 written. addr: "));
        Serial.println(addr_);
    }

    static bool ReadFit64FromI2C_eeprom(uint16_t& addr_, std::vector<float64_t>& fit64) {
        I2C_eeprom i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512);
        uint8_t arraysize = i2C_eeprom.readByte(addr_);
        Serial.print(F("Read Arraysize: "));
        Serial.println(arraysize);
        if (arraysize == 0 || arraysize == 0xFF || arraysize > 20) {
            return false;
        }
        addr_ += 1;
        fit64 = std::vector<float64_t>(arraysize);
        uint8_t datasize = fit64.size() * sizeof(float64_t);
        i2C_eeprom.readBlock(addr_, (uint8_t*)fit64.begin(), datasize);
        addr_ += datasize;
        Serial.print(F("fit64 read. addr: "));
        Serial.println(addr_);
        return true;
    }

    static void WriteFit64ToI2C_eeprom()
    {
        I2C_eeprom i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512);
        i2C_eeprom.begin();
        if (i2C_eeprom.isConnected()) {
            uint16_t addr(0);
            std::vector<std::vector<float64_t>*> fit64s = { &fit64RV45_l, &fit64RV45_r };
            for (std::vector<std::vector<float64_t>*>::iterator i = fit64s.begin(); fit64s.end() != i; i++) {
                if (!(*i)->empty()) {
                    WriteFit64ToI2C_eeprom(addr, **i);
                }
            }

        }
    }

    static bool ReadFit64FromI2C_eeprom(void)
    {
        I2C_eeprom i2C_eeprom(0x50, I2C_DEVICESIZE_24LC512);
        i2C_eeprom.begin();
        if (i2C_eeprom.isConnected()) {
            uint16_t addr(0);
            std::vector<std::vector<float64_t>*> fit64s = { &fit64RV45_l, &fit64RV45_r };
            for (std::vector<std::vector<float64_t>*>::iterator i = fit64s.begin(); fit64s.end() != i; i++) {
                bool result = ReadFit64FromI2C_eeprom(addr, **i);
                if (!result) {
                    Serial.println("ReadFit64FromI2C_eeprom Error");
                    return false;
                }
            }
            return true;
        }
        return false;
    }



    static void Device1(void) {
        const char* fit64_flashTable[] PROGMEM = {
            "-6.162314822380227e-08",
            "-4.378308439972472e-06",
            "-0.00011688793870179859",
            "-0.0013040839584018702",
            "-0.003870823753524697",
            "-0.020528115676117634",
            "-0.5461083074144886",
            "16.406533912931025",
            "254.83884893125213"
        };
        constexpr auto fit64_size = sizeof(fit64_flashTable) / sizeof(const char*);
        fit64 = std::vector <float64_t>(fit64_size);
        for (int i = 0; i != fit64.size(); i++) {
            fit64[fit64_size - i - 1] = fp64_atof(const_cast<char*>(fit64_flashTable[i]));
        }
        fit64RV45_l = std::vector <float64_t>(15);
        fit64RV45_l[14] = fp64_atof(String(F("-1.677923585110501e-36")).c_str());
        fit64RV45_l[13] = fp64_atof(String(F("1.2560653304131632e-32")).c_str());
        fit64RV45_l[12] = fp64_atof(String(F("-4.227894224574707e-29")).c_str());
        fit64RV45_l[11] = fp64_atof(String(F("8.45064378915242e-26")).c_str());
        fit64RV45_l[10] = fp64_atof(String(F("-1.115804991376365e-22")).c_str());
        fit64RV45_l[9] = fp64_atof(String(F("1.0244379036302585e-19")).c_str());
        fit64RV45_l[8] = fp64_atof(String(F("-6.705683940884093e-17")).c_str());
        fit64RV45_l[7] = fp64_atof(String(F("3.158402453382945e-14")).c_str());
        fit64RV45_l[6] = fp64_atof(String(F("-1.0682418823283996e-11")).c_str());
        fit64RV45_l[5] = fp64_atof(String(F("2.566411400535807e-09")).c_str());
        fit64RV45_l[4] = fp64_atof(String(F("-4.3066302928320864e-07")).c_str());
        fit64RV45_l[3] = fp64_atof(String(F("4.9748269797742075e-05")).c_str());
        fit64RV45_l[2] = fp64_atof(String(F("-0.004030972379686488")).c_str());
        fit64RV45_l[1] = fp64_atof(String(F("0.28074280452148")).c_str());
        fit64RV45_l[0] = fp64_atof(String(F("-29.355779672332574")).c_str());

        fit64RV45_r = std::vector <float64_t>(15);
        fit64RV45_r[14] = fp64_atof(String(F("-2.1287535929115348e-36")).c_str());
        fit64RV45_r[13] = fp64_atof(String(F("1.5748997961393054e-32")).c_str());
        fit64RV45_r[12] = fp64_atof(String(F("-5.235180407213225e-29")).c_str());
        fit64RV45_r[11] = fp64_atof(String(F("1.0326522313620901e-25")).c_str());
        fit64RV45_r[10] = fp64_atof(String(F("-1.3446707593535264e-22")).c_str());
        fit64RV45_r[9] = fp64_atof(String(F("1.2167590034249164e-19")).c_str());
        fit64RV45_r[8] = fp64_atof(String(F("-7.84515584960287e-17")).c_str());
        fit64RV45_r[7] = fp64_atof(String(F("3.6376381048261166e-14")).c_str());
        fit64RV45_r[6] = fp64_atof(String(F("-1.2103995770781873e-11")).c_str());
        fit64RV45_r[5] = fp64_atof(String(F("2.8579468570036992e-09")).c_str());
        fit64RV45_r[4] = fp64_atof(String(F("-4.704431961655748e-07")).c_str());
        fit64RV45_r[3] = fp64_atof(String(F("5.3107745275726005e-05")).c_str());
        fit64RV45_r[2] = fp64_atof(String(F("-0.004178304781667345")).c_str());
        fit64RV45_r[1] = fp64_atof(String(F("0.28128607980332293")).c_str());
        fit64RV45_r[0] = fp64_atof(String(F("-29.084007135068532")).c_str());
    }

    static void Device2(void) {
        const char* fit64_flashTable[] PROGMEM = {
            "2.6008901174857894e-05",
            "0.001382974224325812",
            "0.025824261833570294",
            "0.1628642044710132",
            "0.05232081299714197",
            "17.632868585802612",
            "255.28888529165388"
        };
        constexpr auto fit64_size = sizeof(fit64_flashTable) / sizeof(const char*);
        fit64 = std::vector <float64_t>(fit64_size);
        for (int i = 0; i != fit64.size(); i++) {
            fit64[fit64_size - i - 1] = fp64_atof(const_cast<char*>(fit64_flashTable[i]));
        }

        fit64RV45_l = std::vector <float64_t>(15);
        fit64RV45_l[14] = fp64_atof(String(F("-1.3012260559125937e-35")).c_str());
        fit64RV45_l[13] = fp64_atof(String(F("9.494262059115952e-32")).c_str());
        fit64RV45_l[12] = fp64_atof(String(F("-3.1268682744213197e-28")).c_str());
        fit64RV45_l[11] = fp64_atof(String(F("6.146799573438062e-25")).c_str());
        fit64RV45_l[10] = fp64_atof(String(F("-8.037299976392854e-22")).c_str());
        fit64RV45_l[9] = fp64_atof(String(F("7.374707899901929e-19")).c_str());
        fit64RV45_l[8] = fp64_atof(String(F("-4.88300808491803e-16")).c_str());
        fit64RV45_l[7] = fp64_atof(String(F("2.3633748281711946e-13")).c_str());
        fit64RV45_l[6] = fp64_atof(String(F("-8.379425770217733e-11")).c_str());
        fit64RV45_l[5] = fp64_atof(String(F("2.1609842284031454e-08")).c_str());
        fit64RV45_l[4] = fp64_atof(String(F("-3.986407205204476e-06")).c_str());
        fit64RV45_l[3] = fp64_atof(String(F("0.000511188364967593")).c_str());
        fit64RV45_l[2] = fp64_atof(String(F("-0.04357093118373209")).c_str());
        fit64RV45_l[1] = fp64_atof(String(F("2.329489728986675")).c_str());
        fit64RV45_l[0] = fp64_atof(String(F("-82.46352504265981")).c_str());

        fit64RV45_r = std::vector <float64_t>(15);
        fit64RV45_r[14] = fp64_atof(String(F("-1.4436231649486664e-35")).c_str());
        fit64RV45_r[13] = fp64_atof(String(F("1.0489683655067018e-31")).c_str());
        fit64RV45_r[12] = fp64_atof(String(F("-3.4395003093908697e-28")).c_str());
        fit64RV45_r[11] = fp64_atof(String(F("6.729451414975763e-25")).c_str());
        fit64RV45_r[10] = fp64_atof(String(F("-8.754188106097561e-22")).c_str());
        fit64RV45_r[9] = fp64_atof(String(F("7.987655610880187e-19")).c_str());
        fit64RV45_r[8] = fp64_atof(String(F("-5.256314114862736e-16")).c_str());
        fit64RV45_r[7] = fp64_atof(String(F("2.5266654527916786e-13")).c_str());
        fit64RV45_r[6] = fp64_atof(String(F("-8.89000755255239e-11")).c_str());
        fit64RV45_r[5] = fp64_atof(String(F("2.2731008508940537e-08")).c_str());
        fit64RV45_r[4] = fp64_atof(String(F("-4.15342530472971e-06")).c_str());
        fit64RV45_r[3] = fp64_atof(String(F("0.000527040998067761")).c_str());
        fit64RV45_r[2] = fp64_atof(String(F("-0.04441480169308953")).c_str());
        fit64RV45_r[1] = fp64_atof(String(F("2.3465342934877804")).c_str());
        fit64RV45_r[0] = fp64_atof(String(F("-82.24086926718981")).c_str());
    }


    static std::vector<float64_t> fit64RV45_l;
    static std::vector<float64_t> fit64RV45_r;
    static std::vector<float64_t> fit64;

};

Relay System::muteRelay(Relay(28, true));
Relay System::calibrationRelay(Relay(26));

std::vector<float64_t> System::fit64;
std::vector<float64_t> System::fit64RV45_l(0);
std::vector<float64_t> System::fit64RV45_r(0);
