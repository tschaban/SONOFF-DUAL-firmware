/*
 SONOFF DUAL: firmware
 More info: https://github.com/tschaban/SONOFF-DUAL-firmware
 LICENCE: http://opensource.org/licenses/MIT
 2017-02-12 tschaban https://github.com/tschaban
*/

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <DallasTemperature.h>
#include <OneWire.h>


#include "Streaming.h"
#include "sonoff-configuration.h"
#include "sonoff-core.h"
#include "sonoff-led.h"
#include "sonoff-eeprom.h"
#include "sonoff-relay.h"
#include "sonoff-button.h"
#include "sonoff-ota.h"

DEFAULTS sonoffDefault;
SONOFFCONFIG Configuration; 

WiFiClient esp;
PubSubClient Mqtt(esp);

ESP8266WebServer server(80);
DNSServer dnsServer;
ESP8266HTTPUpdateServer httpUpdater;
OneWire wireProtocol(SENSOR_DS18B20);
DallasTemperature DS18B20(&wireProtocol);

SonoffEEPROM    Eeprom;
SonoffRelay     Relay;
SonoffButton    Button;
SonoffLED       Led;
Sonoff          Sonoff;

void setup() {
  Serial.begin(19200);
  delay(10);

  Serial.println();

  Configuration = Eeprom.getConfiguration();

 
  Serial << endl << "Configuration: " << endl;
  Serial << " - Version: " << Configuration.version << endl;
  Serial << " - Language: " << Configuration.language << endl;
  Serial << " - Switch mode: " << Configuration.mode << endl;
  Serial << " - Host name: " << Configuration.device_name << endl;
  Serial << " - WiFi SSID: " << Configuration.wifi_ssid << endl;
  Serial << " - WiFi Password: " << Configuration.wifi_password << endl;
  Serial << " - MQTT Host: " << Configuration.mqtt_host << endl;
  Serial << " - MQTT Port: " << Configuration.mqtt_port << endl;
  Serial << " - MQTT User: " << Configuration.mqtt_user << endl;
  Serial << " - MQTT Password: " << Configuration.mqtt_password << endl;
  Serial << " - MQTT Topic: " << Configuration.mqtt_topic <<  endl;  
  Serial << " - Relay 1 name: " << Configuration.relay_1_name << endl;
  Serial << " - Relay 2 name: " << Configuration.relay_2_name << endl;
  Serial << " - Relay 1 state: " << Eeprom.getRelayState(RELAY_FIRST) << endl;
  Serial << " - Relay 2 state: " << Eeprom.getRelayState(RELAY_SECOND) << endl;
  Serial << " - Post crash relay 1 state: " << Eeprom.getRelayStateAfterPowerRestored(RELAY_FIRST) << endl;
  Serial << " - Post crash relay 2 state: " << Eeprom.getRelayStateAfterPowerRestored(RELAY_SECOND) << endl;
  Serial << " - DS18B20 present: " << Configuration.ds18b20_present << endl;
  Serial << " - Temp correctin: " << Configuration.ds18b20_correction << endl;
  Serial << " - Temp interval: " << Configuration.ds18b20_interval << endl;

   
  Sonoff.run(); 
  
}

void loop() {
  Sonoff.listener();
}
