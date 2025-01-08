#include <Arduino.h>
#include "Firefighter.h"
#include "Comms.h"
#include <cmath>
#include <unordered_map>
#include <string>
#include "hardware_config.h"

using namespace std;

Firefighter firefighter;
Comms* comms = nullptr; 

const unsigned long DEBOUNCE_DELAY = 1000; // Debounce delay in milliseconds

bool noButtonPressed = false;
bool helpButtonPressed = false;
bool yesButtonPressed = false;

unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;

volatile bool noButtonRaw = false;
volatile bool helpButtonRaw = false;
volatile bool yesButtonRaw = false;

void IRAM_ATTR NoButton() { noButtonRaw = true; }
void IRAM_ATTR HelpButton() { helpButtonRaw = true; }
void IRAM_ATTR YesButton() { yesButtonRaw = true; }

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
  delay(1000);

  comms = new Comms(&firefighter);

  //firefighter.registerSerialOutput(&comms->serialOutPutQueue);
  firefighter.registerMeshOutput(&comms->meshOutputQueue);
  comms->start();

  // Init hardware, buttons and TFT display and LED
  hardwareInit();

  // Attach interrupts to the button pins
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), NoButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2), HelpButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_3), YesButton, FALLING);
}

// inget görs här, aktiviteter sköts i freeRTOS tasks
void loop() 
{
  checkDebouncedButton(noButtonRaw, lastDebounceTime1, noButtonPressed);
  checkDebouncedButton(helpButtonRaw, lastDebounceTime2, helpButtonPressed);
  checkDebouncedButton(yesButtonRaw, lastDebounceTime3, yesButtonPressed);

  if (noButtonPressed)
  {
    noButtonPressed = false;
    printToDisplay("No pressed");
    //comms->enqueueMeshOutput(Message(firefighter.leaderID, "No")); 
    firefighter.enqueueMeshOutput(Message(firefighter.leaderID, "No"));
    setLEDOff();
  }

  if (helpButtonPressed) 
  {
    helpButtonPressed = false;
    printToDisplay("Help requested");
    firefighter.positionsList.clear();  // Rensa listan över positioner
    //comms->enqueueMeshOutput(Message(0, "ReqPos")); 
    //firefighter.enqueueMeshOutput(Message(0, "ReqPos"));
    firefighter.sendHelpRequest();
  }

  if (yesButtonPressed) 
  {
    yesButtonPressed = false;
    printToDisplay("Yes pressed");
    //comms->enqueueMeshOutput(Message(firefighter.leaderID, "Yes"));
    //firefighter.enqueueMeshOutput(Message(firefighter.leaderID, "Yes"));
    firefighter.startMission();
    setLEDOff(); 
  }  
}
