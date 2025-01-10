#include "BridgeComms.h"


void setup() 
{
    delay(1000);
    static BridgeComms comms;
    comms.start();     
}

void loop() {}  // inget görs här, aktiviteter sköts i freeRTOS tasks
