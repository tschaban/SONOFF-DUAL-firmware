/*
  SONOFF DUAL: firmware
  More info: https://github.com/tschaban/SONOFF-DUAL-firmware
  LICENCE: http://opensource.org/licenses/MIT
  2017-02-12 tschaban https://github.com/tschaban
*/

#include "sonoff-relay.h"

SonoffRelay::SonoffRelay() {
  init(RELAY_FIRST);
  init(RELAY_SECOND);
}

void SonoffRelay::init(byte id) {
  if (Eeprom.getRelayStartState(id) == DEFAULT_RELAY_ON) {
    on(id);
  } else if (Eeprom.getRelayStartState(id) == DEFAULT_RELAY_OFF) {
    off(id);
  } else if (Eeprom.getRelayStartState(id) == DEFAULT_RELAY_LAST_KNOWN) {
    if (Eeprom.getRelayState(id) == 1) {
      on(id);
    } else {
      off(id);
    }
  }
}

/* Set relay to ON */
void SonoffRelay::on(byte relayID) {
  if (relayID == RELAY_BOTH) {
    Relay_1 = true;
    Relay_2 = true;
  } else if (relayID == RELAY_FIRST) {
    Relay_1 = true;
  } else if (relayID == RELAY_SECOND) {
    Relay_2 = true;
  }

  setRelay();
  publish(relayID);

  Serial << "Relay: " << relayID << " set to ON" << endl;
  Led.blink();
}

/* Set relay to OFF */
void SonoffRelay::off(byte relayID) {
  if (relayID == RELAY_BOTH) {
    Relay_1 = false;
    Relay_2 = false;
  } else if (relayID == RELAY_FIRST) {
    Relay_1 = false;
  } else if (relayID == RELAY_SECOND) {
    Relay_2 = false;
  }

  setRelay();
  publish(relayID);

  Serial << "Relay: " << relayID << " set to OFF" << endl;
  Led.blink();
}

/* Toggle relay */
void SonoffRelay::toggle(byte relayID) {
  if (relayID == RELAY_BOTH) {
    Relay_1 = !Relay_1;
    Relay_2 = !Relay_2;
  } else if (relayID == RELAY_FIRST) {
    Relay_1 = !Relay_1;
  } else if (relayID == RELAY_SECOND) {
    Relay_2 = !Relay_2;
  }
  setRelay();
  publish(relayID);

  Serial << "Relay: " << relayID << " toggled" << endl;
  Led.blink();
}

void SonoffRelay::publish(byte relayID) {
  char  mqttString[50];
  if (relayID == RELAY_FIRST || relayID == RELAY_BOTH) {
    publishState(RELAY_FIRST);
  }

  if (relayID == RELAY_SECOND || relayID == RELAY_BOTH) {
    publishState(RELAY_SECOND);
  }
}

void SonoffRelay::publishState(byte id) {
    char  mqttString[50];
    sprintf(mqttString, "%s%s/state", Configuration.mqtt_topic,id==RELAY_FIRST?Configuration.relay_1_name:Configuration.relay_2_name);
    Eeprom.saveRelayState(id,id==RELAY_FIRST?Relay_1?1:0:Relay_2?1:0);
    Mqtt.publish(mqttString, id==RELAY_FIRST?Relay_1?"ON":"OFF":Relay_2?"ON":"OFF");
    Serial << "Publish: " << mqttString << endl;
}

void SonoffRelay::setRelay () {
  byte address = 0;
  if (Relay_1) address++;
  if (Relay_2) address += 2;
  Serial.write(0xA0);
  Serial.write(0x04);
  Serial.write(address);
  Serial.write(0xA1);
  Serial.flush();
}


