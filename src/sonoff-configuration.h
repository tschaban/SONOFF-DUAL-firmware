/*
 SONOFF DUAL: firmware
 More info: https://github.com/tschaban/SONOFF-DUAL-firmware
 LICENCE: http://opensource.org/licenses/MIT
 2017-02-12 tschaban https://github.com/tschaban
*/


#ifndef _sonoff_configuration_h
#define _sonoff_configuration_h

#define BUTTON 0
#define GPIO_LED 13

/* Configuration parameters */
#define   CONNECTION_WAIT_TIME 500
#define   MODE_SWITCH 0
#define   MODE_CONFIGURATION 1
#define   MODE_ACCESSPOINT 2

/* Values for EEPROM Parameter Relay state after crash */
#define   DEFAULT_RELAY_ON 1
#define   DEFAULT_RELAY_OFF 2
#define   DEFAULT_RELAY_LAST_KNOWN 3
#define   DEFAULT_RELAY_SERVER 4

#define   RELAY_BOTH 0
#define   RELAY_FIRST 1
#define   RELAY_SECOND 2

#define   RELAY_OFF 0
#define   RELAY_ON 1

struct DEFAULTS {
  char          version[6] = "0.1.1";
  char          language[3] = "en";
  unsigned int  mqtt_port = 1883;
  float         temp_correction = 0;
  unsigned int  temp_interval = 600;
  boolean       temp_present = false; 
  uint8_t       relay_state_after_power_restored = 3;
  uint8_t       relay_state_after_connection_restored = 4;  
  char          relay_1_name[16] = "1";
  char          relay_2_name[16] = "2";     
 };


struct SONOFFCONFIG {
  char          version[6];
  char          language[3];
  char          device_name[16] = {0};

  uint8_t       mode;
  char          wifi_ssid[32];
  char          wifi_password[32];

  char          mqtt_host[32];
  unsigned int  mqtt_port;
  char          mqtt_user[16];
  char          mqtt_password[16];  
  char          mqtt_topic[32];
  
  char          relay_1_name[16];
  char          relay_2_name[16];          


  float         ds18b20_correction;
  unsigned int  ds18b20_interval;
  boolean       ds18b20_present;
};


#endif
