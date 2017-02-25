/*
 SONOFF DUAL: firmware
 More info: https://github.com/tschaban/SONOFF-DUAL-firmware
 LICENCE: http://opensource.org/licenses/MIT
 2017-02-12 tschaban https://github.com/tschaban
*/

#include "sonoff-eeprom.h"

SonoffEEPROM::SonoffEEPROM() {
  EEPROM.begin(EEPROM_size);

  /* If version not set it assumes first launch */
  if (read(0,8)[0] == '\0')  {
    erase();
    saveMode(1);
  }
}


SONOFFCONFIG SonoffEEPROM::getConfiguration() {
  SONOFFCONFIG _temp;

  read(0,5).toCharArray(_temp.version,sizeof(_temp.version));  
  _temp.mode = read(21, 1).toInt();
  
  // If there is no version in EPPROM this a first launch 
  if(_temp.version[0] == '\0')  {
    erase();
    _temp.mode = MODE_ACCESSPOINT;
  }

  read(5, 16).toCharArray(_temp.device_name, sizeof(_temp.device_name));
  read(22, 2).toCharArray(_temp.language, sizeof(_temp.language));


  /* Reading WiFi Parameters */
  read(24, 32).toCharArray(_temp.wifi_ssid, sizeof(_temp.wifi_ssid));
  read(56, 32).toCharArray(_temp.wifi_password, sizeof(_temp.wifi_password));  

  /* Reading MQTT Parameters */
  read(88, 32).toCharArray(_temp.mqtt_host, sizeof(_temp.mqtt_host));
  _temp.mqtt_port = read(120, 5).toInt();      
  read(125, 16).toCharArray(_temp.mqtt_user, sizeof(_temp.mqtt_user));
  read(141, 16).toCharArray(_temp.mqtt_password, sizeof(_temp.mqtt_password));
  read(157, 32).toCharArray(_temp.mqtt_topic, sizeof(_temp.mqtt_topic));  

  /* Reading Relay Parameters */
  read(189, 16).toCharArray(_temp.relay_1_name, sizeof(_temp.relay_1_name));
  read(205, 16).toCharArray(_temp.relay_2_name, sizeof(_temp.relay_2_name));

  /* Reading DS18B20 Parameters */
  _temp.ds18b20_present = isDS18B20Present();
  _temp.ds18b20_correction = DS18B20Correction();
  _temp.ds18b20_interval = DS18B20ReadInterval();

  return _temp;
}

boolean SonoffEEPROM::isDS18B20Present() {
  return (read(227, 1).toInt() == 1 ? true : false);
}

float SonoffEEPROM::DS18B20Correction() {
  return read(228, 6).toFloat();
}

unsigned int SonoffEEPROM::DS18B20ReadInterval() {
  return read(234, 5).toInt();
}


uint8_t SonoffEEPROM::getRelayState(byte id) {
  if (id==RELAY_FIRST) 
    return read(221, 1).toInt();
  else
    return read(222, 1).toInt();
    
}

uint8_t SonoffEEPROM::getRelayStateAfterPowerRestored(byte id) {
  if (id==RELAY_FIRST) 
    return read(223, 1).toInt();
  else
    return read(224, 1).toInt();
}

uint8_t SonoffEEPROM::getRelayStateAfterConnectionRestored(byte id) {
  if (id==RELAY_FIRST) 
    return read(225, 1).toInt();
  else
    return read(226, 1).toInt();
}

void SonoffEEPROM::saveVersion(String in) {
  write(0, 5, in);
}

void SonoffEEPROM::saveDeviceName(String in) {
  write(5, 16, in);
}

void SonoffEEPROM::saveLanguage(String in) {
  write(22,2,in);
}
  
void SonoffEEPROM::saveMode(int in) {
  write(21, 1, String(in));
    Serial << " - Switch mode: " << String(in)<< endl;
}

void SonoffEEPROM::saveTemperatureCorrection(float in) {
  write(228, 6, String(in));
}

void SonoffEEPROM::saveTemperatureInterval(unsigned int in) {
  write(234, 5, String(in));
}

void SonoffEEPROM::saveTemperatureSensorPresent(unsigned int in) {
  write(227, 1, String(in));
}

void SonoffEEPROM::saveRelayState(byte id, unsigned int in) {
  if (id==RELAY_FIRST) 
     write(221, 1, String(in));
  else 
     write(222, 1, String(in));   
}

void SonoffEEPROM::saveRelayStateAfterPowerRestored(byte id, unsigned int in) {
  if (id==RELAY_FIRST) 
     write(223, 1, String(in));
  else 
     write(224, 1, String(in)); 
}

void SonoffEEPROM::saveRelayStateAfterConnectionRestored(byte id, unsigned int in) {
  if (id==RELAY_FIRST) 
     write(225, 1, String(in));
  else 
     write(226, 1, String(in)); 
}

void SonoffEEPROM::saveRelayName(byte id, String in) {
  if (id==RELAY_FIRST) 
     write(189, 16, in);
  else 
     write(205, 16, in); 
}

void SonoffEEPROM::saveWiFiSSID(String in) {
  write(24, 32, in);
}

void SonoffEEPROM::saveWiFiPassword(String in) {
  write(56, 32, in);
}

void SonoffEEPROM::saveMQTTHost(String in) {
  write(88, 32, in);
}

void SonoffEEPROM::saveMQTTPort(unsigned int in) {
  write(120, 5, String(in));
}

void SonoffEEPROM::saveMQTTUser(String in) {
  write(125, 16, in);
}

void SonoffEEPROM::saveMQTTPassword(String in) {
  write(141, 16, in);
}

void SonoffEEPROM::saveMQTTTopic(String in) {
  write(157, 32, in);
}

void SonoffEEPROM::erase() {
  clear(0, EEPROM_size);
  setDefaults();
}

void SonoffEEPROM::setDefaults() {
  
  char _id[6] = {0};
  char _device_name[16] = {0};
  char _mqtt_topic[32] = {0};

  sprintf(_id, "%06X", ESP.getChipId());
  sprintf(_device_name, "SONOFF_%s", _id);
  sprintf(_mqtt_topic, "/sonoff/%s/", _id);

  write(5, 16, _device_name);

  saveVersion(sonoffDefault.version);

  saveTemperatureCorrection(sonoffDefault.temp_correction);
  saveTemperatureInterval(sonoffDefault.temp_interval);
  saveTemperatureSensorPresent(sonoffDefault.temp_present);

  saveMQTTTopic(_mqtt_topic);
  saveMQTTPort(sonoffDefault.mqtt_port);

  saveMode(MODE_SWITCH);

  saveRelayName(RELAY_FIRST,sonoffDefault.relay_1_name);
  saveRelayName(RELAY_SECOND,sonoffDefault.relay_2_name);
  
  saveRelayState(RELAY_FIRST,0);
  saveRelayState(RELAY_SECOND,0);
  
  saveRelayStateAfterPowerRestored(RELAY_FIRST,sonoffDefault.relay_state_after_power_restored);
  saveRelayStateAfterConnectionRestored(RELAY_FIRST,sonoffDefault.relay_state_after_connection_restored);  
  saveRelayStateAfterPowerRestored(RELAY_SECOND,sonoffDefault.relay_state_after_power_restored);
  saveRelayStateAfterConnectionRestored(RELAY_SECOND,sonoffDefault.relay_state_after_connection_restored); 
  
  saveLanguage(sonoffDefault.language); 
}


void SonoffEEPROM::write(int address, int size, String in) {
  clear(address, size);
  for (int i = 0; i < in.length(); ++i)
  {
    EEPROM.write(address + i, in[i]);
  }
  EEPROM.commit();
}

String SonoffEEPROM::read(int address, int size) {
  String _return;
  for (int i = address; i < address + size; ++i)
  {
    if (EEPROM.read(i) != 255) {
      _return += char(EEPROM.read(i));
    }
  }
  return _return;
}

void SonoffEEPROM::clear(int address, int size) {
  for (int i = 0; i < size; ++i) {
    EEPROM.write(i + address, 255);
  }
  EEPROM.commit();
}

