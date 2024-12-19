#include <Arduino.h>
#include "Tile.h"
#include "Firefighter.h"
#include "hardware_config.h"

// Definiera knapptryckningar i en array för skalbarhet
Firefighter firefighter;

const unsigned long DEBOUNCE_DELAY = 1000;
bool buttonPressed[3] = {false, false, false};
volatile bool buttonRaw[3] = {false, false, false};
unsigned long lastDebounceTime[3] = {0, 0, 0};

// Attach interrupt-funktioner för varje knapp
void IRAM_ATTR ButtonHandler(int buttonIndex) 
{
  buttonRaw[buttonIndex] = true;
}

void checkDebouncedButton(volatile bool& buttonRaw, unsigned long& lastDebounceTime, bool& buttonPressed) 
{
  if (buttonRaw) 
  {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime > DEBOUNCE_DELAY) 
    {
      buttonPressed = true;
      lastDebounceTime = currentTime;
    }
    buttonRaw = false;
  }
}

void setup() 
{
  Serial.begin(115200);
  Serial.setTimeout(50);
  
  // Init hardware, buttons and TFT display and LED
  hardwareInit();
  
  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), []() { ButtonHandler(0); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2), []() { ButtonHandler(1); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_3), []() { ButtonHandler(2); }, FALLING);
}

void loop() 
{
  for (int i = 0; i < 3; i++) 
  {
    checkDebouncedButton(buttonRaw[i], lastDebounceTime[i], buttonPressed[i]);
  }

  if (buttonPressed[0]) 
  {
    buttonPressed[0] = false;
    printToDisplay("No pressed");
    firefighter.comms.meshPush("No", firefighter.leaderID);
    setLEDOff();
  }

  if (buttonPressed[1]) 
  {
    buttonPressed[1] = false;
    printToDisplay("Help requested");
    firefighter.comms.meshPush("ReqPos", 0);
  }

  if (buttonPressed[2]) 
  {
    buttonPressed[2] = false;
    printToDisplay("Yes pressed");
    firefighter.startMission();
    setLEDOff();
  }  
}
