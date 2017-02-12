/*
  SONOFF DUAL: firmware
  More info: https://github.com/tschaban/SONOFF-DUAL-firmware
  LICENCE: http://opensource.org/licenses/MIT
  2017-02-12 tschaban https://github.com/tschaban
*/

#include "sonoff-relay.h"

SonoffRelay::SonoffRelay() {

  if (Eeprom.getRelayStartState(RELAY_FIRST) == DEFAULT_RELAY_ON) {
    on(RELAY_FIRST);
  } else if (Eeprom.getRelayStartState(RELAY_FIRST) == DEFAULT_RELAY_OFF) {
    off(RELAY_FIRST);
  } else if (Eeprom.getRelayStartState(RELAY_FIRST) == DEFAULT_RELAY_LAST_KNOWN) {
    if (Eeprom.getRelayState(RELAY_FIRST) == 1) {
      on(RELAY_FIRST);
    } else {
      off(RELAY_FIRST);
    }
  }

  if (Eeprom.getRelayStartState(RELAY_SECOND) == DEFAULT_RELAY_ON) {
    on(RELAY_FIRST);
  } else if (Eeprom.getRelayStartState(RELAY_SECOND) == DEFAULT_RELAY_OFF) {
    off(RELAY_FIRST);
  } else if (Eeprom.getRelayStartState(RELAY_SECOND) == DEFAULT_RELAY_LAST_KNOWN) {
    if (Eeprom.getRelayState(RELAY_SECOND) == 1) {
      on(RELAY_SECOND);
    } else {
      off(RELAY_SECOND);
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
}

void SonoffRelay::publish(byte relayID) {
  char  mqttString[50];
  if (relayID == RELAY_FIRST || relayID == RELAY_BOTH) {
    sprintf(mqttString, "%sfirst/state", Configuration.mqtt_topic);
    if (Relay_1) {
      Eeprom.saveRelayState(RELAY_FIRST, 1);
      Mqtt.publish(mqttString, "ON");
    } else {
      Eeprom.saveRelayState(RELAY_FIRST, 0);
      Mqtt.publish(mqttString, "OFF");
    }
  }

  if (relayID == RELAY_SECOND || relayID == RELAY_BOTH) {
    sprintf(mqttString, "%ssecond/state", Configuration.mqtt_topic);
    if (Relay_2) {
      Eeprom.saveRelayState(RELAY_SECOND, 1);
      Mqtt.publish(mqttString, "ON");
    } else {
      Eeprom.saveRelayState(RELAY_SECOND, 0);
      Mqtt.publish(mqttString, "OFF");
    }
  }

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


