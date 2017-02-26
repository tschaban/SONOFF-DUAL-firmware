/*
 SONOFF DUAL: firmware
 More info: https://github.com/tschaban/SONOFF-DUAL-firmware
 LICENCE: http://opensource.org/licenses/MIT
 2017-02-12 tschaban https://github.com/tschaban
*/

#ifndef _sonoff_EEPROM_h
#define _sonoff_EEPROM_h

#include <EEPROM.h>
#include "sonoff-configuration.h"

#define EEPROM_SIZE 512

class SonoffEEPROM
{
  private:
    int EEPROM_size = 512;
    void write(int address, int size, String in);
    String read(int address, int size);
    void clear(int start, int size);
    void setDefaults();

  public:
    SonoffEEPROM();

    SONOFFCONFIG getConfiguration();

    boolean isDebuggable();
   
    uint8_t getRelayState(byte id);
 
    uint8_t getRelayStateAfterPowerRestored(byte id);
    uint8_t getRelayStateAfterConnectionRestored(byte id);

    boolean isDS18B20Present();
    float   DS18B20Correction();
    unsigned int DS18B20ReadInterval();

    void saveDeviceName(String in);
    void saveVersion(String in);
    void saveLanguage(String in);

    void saveMode(int in); /* Look at MODE_... defined */
    void saveWiFiSSID(String in);
    void saveWiFiPassword(String in);
    
    void saveMQTTHost(String in);
    void saveMQTTPort(unsigned int in);
    void saveMQTTUser(String in);
    void saveMQTTPassword(String in);
    void saveMQTTTopic(String in);
   
    void saveTemperatureCorrection(float in);
    void saveTemperatureInterval(unsigned int in);
    void saveTemperatureSensorPresent(unsigned int in);
   
    void saveRelayState(byte id,unsigned int in);
    void saveRelayStateAfterPowerRestored(byte id, unsigned int in);
    void saveRelayStateAfterConnectionRestored(byte id, unsigned int in);
    void saveRelayName(byte id, String in);

    void saveDebugMode(boolean in);
    
    void erase();
};
#endif


