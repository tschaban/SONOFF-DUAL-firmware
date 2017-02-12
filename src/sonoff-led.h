/*
 SONOFF DUAL: firmware
 More info: https://github.com/tschaban/SONOFF-DUAL-firmware
 LICENCE: http://opensource.org/licenses/MIT
 2017-02-12 tschaban https://github.com/tschaban
*/

#ifndef _sonoff_led_h
#define _sonoff_led_h

#include <Ticker.h>
#include "sonoff-configuration.h"



class SonoffLED
{
  public:
    SonoffLED();
    void on();
    void off();
    void blink(int t = 100);
    void startBlinking(float t);
    void stopBlinking();

  private:
    Ticker LEDTimer;

};
#endif
