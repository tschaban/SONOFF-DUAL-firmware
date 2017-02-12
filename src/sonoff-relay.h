/*
 SONOFF DUAL: firmware
 More info: https://github.com/tschaban/SONOFF-DUAL-firmware
 LICENCE: http://opensource.org/licenses/MIT
 2017-02-12 tschaban https://github.com/tschaban
*/

#ifndef _sonoff_relay_h
#define _sonoff_relay_h

#include <PubSubClient.h>
#include "sonoff-eeprom.h"
#include "sonoff-led.h"
#include "sonoff-configuration.h"


class SonoffRelay
{
  private:
    boolean Relay_1 = false; // to remember state of relay 1, false = off, true = on
    boolean Relay_2 = false; // to remember state of relay 2, false = off, true = on
    void setRelay ();
  
  public:
    SonoffRelay();
    void on(byte relayID);
    void off(byte relayID);
    void toggle(byte relayID);
    void publish(byte relayID);
};
#endif
