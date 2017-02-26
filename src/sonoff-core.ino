/*
  SONOFF DUAL: firmware
  More info: https://github.com/tschaban/SONOFF-DUAL-firmware
  LICENCE: http://opensource.org/licenses/MIT
  2017-02-12 tschaban https://github.com/tschaban
*/

#include "sonoff-core.h"

Sonoff::Sonoff() {
}

void Sonoff::run() {

  isConfigured();
  postUpgradeCheck();

  if (debug) Serial << " - Switch mode: " << Configuration.mode << endl;

  if (Configuration.mode == MODE_SWITCH) {
    runSwitch();
  } else if (Configuration.mode == MODE_CONFIGURATION) {
    runConfigurationLAN();
  } else if (Configuration.mode == MODE_ACCESSPOINT) {
    runConfigurationAP();
  }
}

void Sonoff::reset() {
  if (debug) Serial << "- ereasing EEPROM" << endl;
  Eeprom.erase();
  ESP.restart();
}

void Sonoff::toggle() {
  if (Configuration.mode == MODE_SWITCH) {
    Eeprom.saveMode(MODE_CONFIGURATION);
  } else {
    Eeprom.saveMode(MODE_SWITCH);
  }
  if (debug) Serial << "Rebooting device" << endl;
  Led.blink();
  delay(10);
  ESP.restart();
}

void Sonoff::connectWiFi() {
  WiFi.hostname(Configuration.device_name);
  WiFi.begin(Configuration.wifi_ssid, Configuration.wifi_password);
  if (debug) Serial << endl << "Connecting to WiFi: " << Configuration.wifi_ssid << endl;
  while (WiFi.status() != WL_CONNECTED) {
    if (debug) Serial << ".";
    delay(CONNECTION_WAIT_TIME);
  }
  if (debug) Serial << endl << " - Connected" << endl;
  if (debug) Serial << " - IP: " << WiFi.localIP() << endl;
}

void Sonoff::connectMQTT() {
  char  mqttString[50];

  Mqtt.setServer(Configuration.mqtt_host, Configuration.mqtt_port);
  Mqtt.setCallback(callbackMQTT);

  sprintf(mqttString, "Sonoff (ID: %s)", Configuration.device_name);
  if (debug) Serial << "Connecting to MQTT : " << Configuration.mqtt_host << ":" << Configuration.mqtt_port << endl;
  if (debug) Serial << " - user : " << Configuration.mqtt_user << endl;
  if (debug) Serial << " - password : " << Configuration.mqtt_password << endl;

  while (!Mqtt.connected()) {
    if (Mqtt.connect(mqttString, Configuration.mqtt_user, Configuration.mqtt_password)) {
      if (debug) Serial << endl << "Connected" << endl;
      sprintf(mqttString, "%s#", Configuration.mqtt_topic);
      Mqtt.subscribe(mqttString);
      if (debug) Serial << " - Subsribed to : " << mqttString << endl;

      /* Post connection relay #1 set up */
      if (Eeprom.getRelayStateAfterConnectionRestored(RELAY_FIRST) == DEFAULT_RELAY_ON && Relay.get(RELAY_FIRST) == RELAY_OFF) {
        Relay.on(RELAY_FIRST);
      } else if (Eeprom.getRelayStateAfterConnectionRestored(RELAY_FIRST) == DEFAULT_RELAY_OFF && Relay.get(RELAY_FIRST) == RELAY_ON) {
        Relay.off(RELAY_FIRST);
      } else if (Eeprom.getRelayStateAfterConnectionRestored(RELAY_FIRST) == DEFAULT_RELAY_LAST_KNOWN) {
        if (Eeprom.getRelayState(RELAY_FIRST) == 0 && Relay.get(RELAY_FIRST) == RELAY_ON) {
          Relay.on(RELAY_FIRST);
        } else if (Eeprom.getRelayState(RELAY_FIRST) == 1 && Relay.get(RELAY_FIRST) == RELAY_OFF) {
          Relay.off(RELAY_FIRST);
        }
      } else  {
        getRelayServerValue(RELAY_FIRST);
      }

      /* Post connection relay #2 set up */
      if (Eeprom.getRelayStateAfterConnectionRestored(RELAY_SECOND) == DEFAULT_RELAY_ON && Relay.get(RELAY_SECOND) == RELAY_OFF) {
        Relay.on(RELAY_SECOND);
      } else if (Eeprom.getRelayStateAfterConnectionRestored(RELAY_SECOND) == DEFAULT_RELAY_OFF && Relay.get(RELAY_SECOND) == RELAY_ON) {
        Relay.off(RELAY_SECOND);
      } else if (Eeprom.getRelayStateAfterConnectionRestored(RELAY_SECOND) == DEFAULT_RELAY_LAST_KNOWN) {
        if (Eeprom.getRelayState(RELAY_SECOND) == 0 && Relay.get(RELAY_SECOND) == RELAY_ON) {
          Relay.on(RELAY_SECOND);
        } else if (Eeprom.getRelayState(RELAY_SECOND) == 1 && Relay.get(RELAY_SECOND) == RELAY_OFF) {
          Relay.off(RELAY_SECOND);
        }
      } else  {
        getRelayServerValue(RELAY_SECOND);
      }


      Led.off();
    } else {
      delay(CONNECTION_WAIT_TIME);
      if (debug) Serial << " - mqtt connection status: " << Mqtt.state();
    }
  }
}

void Sonoff::listener() {
  if (Configuration.mode == MODE_SWITCH) {
    if (!Mqtt.connected()) {
      connectMQTT();
    }
    Mqtt.loop();
  } else if (Configuration.mode == MODE_CONFIGURATION) {
    server.handleClient();
  } else if (Configuration.mode == MODE_ACCESSPOINT) {
    dnsServer.processNextRequest();
    server.handleClient();
  }
}

void Sonoff::setDS18B20Interval( int interval) {
  temperatureTimer.detach();
  temperatureTimer.attach(interval, callbackDS18B20);
}

void Sonoff::publishTemperature(float temperature) {
  char  temperatureString[6];
  char  mqttString[50];

  dtostrf(temperature, 2, 2, temperatureString);
  if (previousTemperature != temperature) {
    if (debug) Serial << " - publishing: " << temperatureString << endl;
    sprintf(mqttString, "%stemperature", Configuration.mqtt_topic);
    Mqtt.publish(mqttString, temperatureString);
    previousTemperature = temperature;
  }
}

void Sonoff::getRelayServerValue(byte id) {
  char  mqttString[50];
  sprintf(mqttString, "%s%s/get", Configuration.mqtt_topic, (id == RELAY_FIRST ? Configuration.relay_1_name : Configuration.relay_2_name));
  if (debug) Serial << endl << " Requesting default relay value";
  Mqtt.publish(mqttString, "defaultState");
  if (debug) Serial << ", completed" << endl;
}

/* Private methods */

void Sonoff::runSwitch() {
  Led.on();
  if (debug) Serial << endl << "Device mode: SWITCH" << endl;
  if (debug) Serial << endl << "Configuring MQTT" << endl;
  connectWiFi();
  if (Configuration.ds18b20_present) {
    if (debug) Serial << endl << "Starting DS18B20" << endl;
    setDS18B20Interval(Configuration.ds18b20_interval);
  } else {
    if (debug) Serial << endl << "DS18B20 not present" << endl;
  }

}

void Sonoff::runConfigurationLAN() {
  Led.on();
  if (debug) Serial << endl << "Device mode: LAN Configuration" << endl;
  WiFi.mode(WIFI_STA);
  connectWiFi();
  startHttpServer();
  // Serial << endl << " - Ready for configuration. Open http://" << WiFi.localIP() << endl << endl;
  Led.startBlinking(0.1);
}

void Sonoff::runConfigurationAP() {
  Led.on();
  if (debug) Serial << endl << "Device mode: Access Point Configuration" << endl;
  if (debug) Serial << " - launching access point" << endl;
  IPAddress apIP(192, 168, 5, 1);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(Configuration.device_name);
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(53, "www.example.com", apIP);
  startHttpServer();
  if (debug) Serial << " - After conecting to WiFi: " << Configuration.device_name << " open: http://192.168.5.1/  " << endl << endl;
  Led.startBlinking(0.1);
}

boolean Sonoff::isConfigured() {
  if (Configuration.wifi_ssid[0] == (char) 0 || Configuration.wifi_password[0] == (char) 0 || Configuration.mqtt_host[0] == (char) 0) {
    if (debug) Serial << endl << "Missing configuration. Going to configuration mode." << endl;
    Eeprom.saveMode(MODE_ACCESSPOINT);
    Configuration = Eeprom.getConfiguration();
  }
}

void Sonoff::postUpgradeCheck() {
  if (String(sonoffDefault.version) != String(Configuration.version)) {
    if (debug) Serial << endl << "SOFTWARE WAS UPGRADED from version : " << Configuration.version << " to " << sonoffDefault.version << endl;
    Eeprom.saveVersion(sonoffDefault.version);
    Configuration = Eeprom.getConfiguration();
  }
}

void callbackDS18B20() {
  SonoffDS18B20 Temperature;
  float temperature = Temperature.get();

  Sonoff.publishTemperature(temperature);
}

/* Callback of MQTT Broker, it listens for messages */
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  char  mqttString[50];
  Led.blink();

  String _topic = String(topic);

  sprintf(mqttString, "%s%s/cmd", Configuration.mqtt_topic, Configuration.relay_1_name);
  String _sonoff_relay_1 = String(mqttString);

  sprintf(mqttString, "%s%s/cmd", Configuration.mqtt_topic, Configuration.relay_2_name);
  String _sonoff_relay_2 = String(mqttString);

  sprintf(mqttString, "%scmd", Configuration.mqtt_topic);
  String _sonoff = String(mqttString);

  if (debug)  Serial << "Received MQTT Message : Topic: " << topic << ", Length: " << length << ", Message: ";

  if (debug)
    for (int i = 0; i < length; i++) {
      Serial << (char)payload[i];
    }


  if (_topic == _sonoff_relay_1) {
    if (length == 2 && (char)payload[0] == 'O' && (char)payload[1] == 'N') { // ON
      Relay.on(RELAY_FIRST);
    } else if (length == 3 && (char)payload[0] == 'O' && (char)payload[2] == 'F') { // OFF
      Relay.off(RELAY_FIRST);
    } else if (length == 11 && (char)payload[0] == 'R' && (char)payload[10] == 'e') { // ReportState
      Relay.publish(RELAY_FIRST);
    }
  }

  if (_topic == _sonoff_relay_2) {
    if (length == 2 && (char)payload[0] == 'O' && (char)payload[1] == 'N') { // ON
      Relay.on(RELAY_SECOND);
    } else if (length == 3 && (char)payload[0] == 'O' && (char)payload[2] == 'F') { // OFF
      Relay.off(RELAY_SECOND);
    } else if (length == 11 && (char)payload[0] == 'R' && (char)payload[10] == 'e') { // ReportState
      Relay.publish(RELAY_SECOND);
    }
  }

  if (_topic == _sonoff) {
    if (length == 15 && (char)payload[1] == 'r' && (char)payload[13] == 'l') { // {reboot:Normal}
      ESP.restart();
    } else if (length == 22 && (char)payload[1] == 'r' && (char)payload[20] == 'n') { // {reboot:Configuration}
      Eeprom.saveMode(MODE_CONFIGURATION);
      ESP.restart();
    } else if (length == 20 && (char)payload[1] == 'r' && (char)payload[18] == 't') { // {reboot:AccessPoint}
      Eeprom.saveMode(MODE_ACCESSPOINT);
      ESP.restart();
    } else if (length == 2 && (char)payload[0] == 'O' && (char)payload[1] == 'N') { // ON
      Relay.on(RELAY_BOTH);
    } else if (length == 3 && (char)payload[0] == 'O' && (char)payload[2] == 'F') { // OFF
      Relay.off(RELAY_BOTH);
    }
  }

  if (debug) Serial << endl;
}

