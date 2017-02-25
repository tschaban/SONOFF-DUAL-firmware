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

  Serial << " - Switch mode: " << Configuration.mode << endl;

  if (Configuration.mode == MODE_SWITCH) {
    runSwitch();
  } else if (Configuration.mode == MODE_CONFIGURATION) {
    runConfigurationLAN();
  } else if (Configuration.mode == MODE_ACCESSPOINT) {
    runConfigurationAP();
  } else {
    Serial << "Internal Application error" << endl;
  }
}

void Sonoff::reset() {
  Serial << "- ereasing EEPROM" << endl;
  Eeprom.erase();
  ESP.restart();
}

void Sonoff::toggle() {
  if (Configuration.mode == MODE_SWITCH) {
    Eeprom.saveMode(MODE_CONFIGURATION);
  } else {
    Eeprom.saveMode(MODE_SWITCH);
  }
  Serial << "Rebooting device" << endl;
  Led.blink();
  delay(10);
  ESP.restart();
}

void Sonoff::connectWiFi() {
  WiFi.hostname(Configuration.device_name);
  WiFi.begin(Configuration.wifi_ssid, Configuration.wifi_password);
  Serial << endl << "Connecting to WiFi: " << Configuration.wifi_ssid << endl;
  while (WiFi.status() != WL_CONNECTED) {
    Serial << ".";
    delay(CONNECTION_WAIT_TIME);
  }
  Serial << endl << " - Connected" << endl;
  Serial << " - IP: " << WiFi.localIP() << endl;
}

void Sonoff::connectMQTT() {
  char  mqttString[50];

  Mqtt.setServer(Configuration.mqtt_host, Configuration.mqtt_port);
  Mqtt.setCallback(callbackMQTT);

  sprintf(mqttString, "Sonoff (ID: %s)", Configuration.device_name);
  Serial << "Connecting to MQTT : " << Configuration.mqtt_host << ":" << Configuration.mqtt_port << endl;
  Serial << " - user : " << Configuration.mqtt_user << endl;
  Serial << " - password : " << Configuration.mqtt_password << endl;

  while (!Mqtt.connected()) {
    if (Mqtt.connect(mqttString, Configuration.mqtt_user, Configuration.mqtt_password)) {
      Serial << endl << "Connected" << endl;
      sprintf(mqttString, "%s#", Configuration.mqtt_topic);
      Mqtt.subscribe(mqttString);
      Serial << " - Subsribed to : " << mqttString << endl;

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
      Serial << " - mqtt connection status: " << Mqtt.state();
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
  } else {
    Serial << "Internal Application Error" << endl;
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
    Serial << " - publishing: " << temperatureString << endl;
    sprintf(mqttString, "%stemperature", Configuration.mqtt_topic);
    Mqtt.publish(mqttString, temperatureString);
    previousTemperature = temperature;
  }
}

void Sonoff::getRelayServerValue(byte id) {
  char  mqttString[50];
  sprintf(mqttString, "%s%s/get", Configuration.mqtt_topic, (id == RELAY_FIRST ? Configuration.relay_1_name : Configuration.relay_2_name));
  Serial << endl << " Requesting default relay value";
  Mqtt.publish(mqttString, "defaultState");
  Serial << ", completed" << endl;
}

/* Private methods */

void Sonoff::runSwitch() {
  Led.on();
  Serial << endl << "Device mode: SWITCH" << endl;
  Serial << endl << "Configuring MQTT" << endl;
  connectWiFi();
  if (Configuration.ds18b20_present) {
    Serial << endl << "Starting DS18B20" << endl;
    setDS18B20Interval(Configuration.ds18b20_interval);
  } else {
    Serial << endl << "DS18B20 not present" << endl;
  }

}

void Sonoff::runConfigurationLAN() {
  Led.on();
  Serial << endl << "Device mode: LAN Configuration" << endl;
  WiFi.mode(WIFI_STA);
  connectWiFi();
  startHttpServer();
  Serial << endl << " - Ready for configuration. Open http://" << WiFi.localIP() << endl << endl;
  Led.startBlinking(0.1);
}

void Sonoff::runConfigurationAP() {
  Led.on();
  Serial << endl << "Device mode: Access Point Configuration" << endl;
  Serial << " - launching access point" << endl;
  IPAddress apIP(192, 168, 5, 1);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(Configuration.device_name);
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(53, "www.example.com", apIP);
  startHttpServer();
  Serial << " - After conecting to WiFi: " << Configuration.device_name << " open: http://192.168.5.1/  " << endl << endl;
  Led.startBlinking(0.1);
}

boolean Sonoff::isConfigured() {
  if (Configuration.wifi_ssid[0] == (char) 0 || Configuration.wifi_password[0] == (char) 0 || Configuration.mqtt_host[0] == (char) 0) {
    Serial << endl << "Missing configuration. Going to configuration mode." << endl;
    Eeprom.saveMode(MODE_ACCESSPOINT);
    Configuration = Eeprom.getConfiguration();
  }
}

void Sonoff::postUpgradeCheck() {
  if (String(sonoffDefault.version) != String(Configuration.version)) {
    Serial << endl << "SOFTWARE WAS UPGRADED from version : " << Configuration.version << " to " << sonoffDefault.version << endl;
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

  Serial << "Received MQTT Message : Topic: " << topic << ", Length: " << length << ", Message: ";

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
    if (length == 6 && (char)payload[0] == 'R' && (char)payload[5] == 't') { // Reboot
      ESP.restart();
    } else if (length == 17 && (char)payload[0] == 'C' && (char)payload[16] == 'e') { // ConfigurationMode
      Eeprom.saveMode(MODE_CONFIGURATION);
      ESP.restart();
    } else if (length == 15 && (char)payload[0] == 'A' && (char)payload[14] == 'e') { // Access Point Mode
      Eeprom.saveMode(MODE_ACCESSPOINT);
      ESP.restart();
    }
  }

  Serial << endl;
}

