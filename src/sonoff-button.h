/*
 SONOFF DUAL: firmware
 More info: https://github.com/tschaban/SONOFF-DUAL-firmware
 LICENCE: http://opensource.org/licenses/MIT
 2017-02-12 tschaban https://github.com/tschaban
*/

#ifndef _sonoff_button_h
#define _sonoff_button_h

#include <Ticker.h>
#include "sonoff-configuration.h"


class SonoffButton
{
  private:  
    Ticker buttonTimer;
    unsigned long counter = 0;
    
  public:
    SonoffButton(); 
    void stop();
    void pressed();
    void reset();
    boolean isPressed();

    boolean accessPointTrigger();
    boolean configurationTrigger();
    boolean relayTrigger();

    
};
#endif
