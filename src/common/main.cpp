#include "Firefighter.h"
#include "Comms.h"

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

static void buttonHandlerTask(void* p)
{
  while(1)
  {
    checkDebouncedButton(noButtonRaw, lastDebounceTime1, noButtonPressed);
    checkDebouncedButton(helpButtonRaw, lastDebounceTime2, helpButtonPressed);
    checkDebouncedButton(yesButtonRaw, lastDebounceTime3, yesButtonPressed);

  if (noButtonPressed)
  {
    noButtonPressed = false;
    printToDisplay("No pressed");
    comms->enqueueMeshOutput(Message(firefighter.leaderID, "No")); 
    setLEDOff();
    firefighter.pendingHelp = false;
    firefighter.tickCounter = 0;
  }

  if (helpButtonPressed) 
  {
    helpButtonPressed = false;
    printToDisplay("Help requested");
    firefighter.leaderID = 0; 
    firefighter.positionsList.clear();  // Rensa listan över positioner
    comms->enqueueMeshOutput(Message(0, "ReqPos")); 
  }

  if (yesButtonPressed) 
  {
    yesButtonPressed = false;
    printToDisplay("Yes pressed");
    comms->enqueueMeshOutput(Message(firefighter.leaderID, "Yes"));
    firefighter.startMission();
    setLEDOff(); 
    firefighter.pendingHelp = false;
    firefighter.tickCounter = 0;
  } 
  vTaskDelay(50 / portTICK_PERIOD_MS); 
  }
}

void setup() 
{
  Serial.begin(115200);
  Serial.setTimeout(50);
  delay(1000);

  comms = new Comms(&firefighter);

  firefighter.registerMeshOutput(&comms->meshOutputQueue);
  comms->start();

  // Init hardware, buttons and TFT display and LED
  hardwareInit();

  // Attach interrupts to the button pins
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), NoButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2), HelpButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_3), YesButton, FALLING);

  xTaskCreatePinnedToCore(buttonHandlerTask, "buttonHandlerTask", 4096, NULL, 1, NULL, 0);
}

// inget görs här, aktiviteter sköts i freeRTOS tasks
void loop() 
{}
