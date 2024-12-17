#include <Arduino.h>
#include "Tile.h"
#include "Firefighter.h"
#include <array>
#include <iostream>
#include <random>
#include <vector>
#include <queue>
#include <cmath>
#include <unordered_map>
#include <string>
//#include "hardware_config.h"
#include "painlessMesh.h"

using namespace std;

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

Firefighter firefighter;
uint32_t bridgeName = 533097877; // namnet på brygga-noden
painlessMesh mesh; //variant på painlessMesh som kan skicka meddelanden till specifika noder baserat på deras egenvalda namn.
uint32_t leaderID;  // ID of the node who sent help request
int missionTargetRow = 0;
int missionTargetColumn = 0;
int positionListCounter = 0;

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

void informBridge(void *pvParameters);  //dek av freertos task funktion som peeriodiskt uppdaterar gui med egenägd info
void informSingleNode(void *pvParameters);
void meshUpdate(void *pvParameters);  //skit i denna, till för pinlessmesh,  freertos task funktion som uppdaterar meshen
void informAllNodes(void *pvParameters);

std::vector<String> tokenize(const String& expression) 
{
    std::vector<String> tokens;
    String token;

    for (int i = 0; i < expression.length(); ++i) {
        char c = expression[i];
        if (c == ' ') {
            if (!token.isEmpty()) {
                tokens.push_back(token);
                token = ""; 
            }
        } else {
            token += c;
        }
    }
    if (!token.isEmpty()) {
        tokens.push_back(token);
    }
    return tokens;
}

bool tryParseInt(const String& str, int& outValue) 
{
    char* endPtr;
    long value = strtol(str.c_str(), &endPtr, 10); // Försök att konvertera strängen

    if (*endPtr == '\0') { // Kontrollera att hela strängen är ett giltigt tal
        outValue = static_cast<int>(value);

        // Kontrollera att värdet ligger inom intervallet för int
        if (value >= INT_MIN && value <= INT_MAX) {
            return true;
        }
    }
    return false; // Parsning misslyckades
}

void handlePositions(uint32_t from, int row, int column)
{  
  float dis = std::sqrt(std::pow(row-firefighter.targetTile->getRow(),2)+std::pow(column-firefighter.targetTile->getColumn(),2));
  firefighter.positionsList.push_back({from, dis}); // Spara nodens position i positionsList
  if (firefighter.positionsList.size() == mesh.getNodeList(false).size()-2) //Check if all nodes anwsered, if true, start sorting
  { 
    std::sort(firefighter.positionsList.begin(), firefighter.positionsList.end(),
    [](const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b) 
    {
      return a.second < b.second; // Compare by distance
    });
    
    positionListCounter = 0;
    
    for (positionListCounter; positionListCounter < 1; positionListCounter++) 
    {
      printToDisplay("Called firefighter: " + String(firefighter.positionsList[positionListCounter].first) + " with distance: " + String(firefighter.positionsList[positionListCounter].second));
      mesh.sendSingle(firefighter.positionsList[positionListCounter].first, "Help " + String(firefighter.targetTile->getRow()) + " " + String(firefighter.targetTile->getColumn()));
    }
  }
}

void handleHelpRequest(uint32_t from, int row, int column)
{
  // TODO: spara id på avsändare.
  leaderID = from;  // Spara id på avsändare
  setLEDColor(0, 0, 255);  // Blå testfärg
  printToDisplay("Help request recieved");
  missionTargetRow = row;
  missionTargetColumn = column;
}

void setup() 
{
  Serial.begin(115200);
  Serial.setTimeout(50);

  // Init hardware, buttons and TFT display and LED
  hardwareInit();

  // Attach interrupts to the button pins
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), NoButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2), HelpButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_3), YesButton, FALLING);

  //mesh.setDebugMsgTypes(ERROR | CONNECTION);
  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);  // Starta meshen

  mesh.onReceive([](uint32_t from, String &msg) 
  {
    printToDisplay("Recieved: " + msg);

    int row = 0;
    int column = 0;
    std::vector<String> tokens = tokenize(msg);

    if (from == bridgeName)
    {
      if (tokens[0] == "Tick") { firefighter.Tick(); }

      else if (tokens.size() == 3 && tryParseInt(tokens[1], row) && tryParseInt(tokens[2], column)) 
      {      
        if (tokens[0] == "Fire")
        {
          firefighter.grid[row][column]->addEvent(Event::FIRE);
        }
        else if (tokens[0] == "Smoke")
        {
          firefighter.grid[row][column]->addEvent(Event::SMOKE);
        }
        else if (tokens[0] == "Victim")
        {
          firefighter.grid[row][column]->addEvent(Event::VICTIM);
        }
        else if (tokens[0] == "Hazmat")
        {
          firefighter.grid[row][column]->addEvent(Event::HAZMAT);
        } 
        else if (tokens[0] == "RemoveVictim")
        {
          firefighter.grid[row][column]->removeEvent(Event::VICTIM);
        }
        else if (tokens[0] == "MaybeDie")
        {
          firefighter.Die(row, column);
        }                
      } 
    }
    else if (tokens.size() == 3 && tryParseInt(tokens[1], row) && tryParseInt(tokens[2], column)) 
    {
      if (tokens[0] == "Pos")
      {
        handlePositions(from, row, column);
      }
      else if (tokens[0] == "Help") 
      {
        handleHelpRequest(from, row, column);        
      }
      else if (tokens[0] == "RemoveHazmat")
      {
        firefighter.grid[row][column]->removeEvent(Event::HAZMAT);
      }
      else if (tokens[0] == "RemoveVictim")
      {
        firefighter.grid[row][column]->removeEvent(Event::VICTIM);
      }
      else if (tokens[0] == "Hazmat")
      {
        firefighter.grid[row][column]->addEvent(Event::HAZMAT);
      } 
    }     
    else 
    {
      if (tokens[0] == "ReqPos") 
      {
        mesh.sendSingle(from, "Pos " + String(firefighter.currentTile->getRow()) + " " + String(firefighter.currentTile->getColumn()));
      }
      else if (tokens[0] == "Yes") 
      {        
        firefighter.teamMembers.push_back(from);
      }
      else if (tokens[0] == "No") 
      { 
        for (int i = 0; i < firefighter.teamMembers.size(); i++) {
            if (firefighter.positionsList[positionListCounter].first == firefighter.teamMembers[i]) {
              i = 0;
              positionListCounter = (positionListCounter + 1) % firefighter.positionsList.size();
            }
        }
        mesh.sendSingle(firefighter.positionsList[positionListCounter].first, "Help " + String(firefighter.targetTile->getRow()) + " " + String(firefighter.targetTile->getColumn()));
        printToDisplay("Called firefighter: " + String(firefighter.positionsList[positionListCounter].first) + " with distance: " + String(firefighter.positionsList[positionListCounter].second));
        positionListCounter = (positionListCounter + 1) % firefighter.positionsList.size();
      }
      else if (tokens[0] == "Arrived")
      {
        firefighter.nbrFirefighters++;
      }
      else if (tokens[0] == "TeamArrived")
      {
        firefighter.TeamArrived();
      } 
    }
  });

  mesh.onChangedConnections([]() {
    printToDisplay("Connection table changed");
  });

  //skapa tasks
  xTaskCreate(meshUpdate, "meshUpdate", 10000, NULL, 1, NULL);
  xTaskCreate(informBridge, "informBridge", 5000, NULL, 1, NULL); 
  xTaskCreate(informSingleNode, "informFirefighters", 5000, NULL, 1, NULL);
  xTaskCreate(informAllNodes, "informAll", 5000, NULL, 1, NULL);
}

// This function is called when a new node connects
void newConnectionCallback(uint32_t nodeId) 
{
    String PositionMsg = "Position:" + String(firefighter.currentTile->getRow()) + "," + String(firefighter.currentTile->getColumn());
    mesh.sendSingle(nodeId, PositionMsg);
}

void informBridge(void *pvParameters) 
{
  while (1) 
  {
     if (!firefighter.messagesToBridge.empty()) 
     {
      String msg = firefighter.messagesToBridge.front();
      printToDisplay("Sent to bridge: " + msg);

      if (!mesh.sendSingle(bridgeName, msg)) 
      {
        printToDisplay("Failed to send message: " + msg);
      }
      firefighter.messagesToBridge.pop();
    }    
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void informSingleNode(void *pvParameters)
{
  while (1) 
  {
     if (!firefighter.messagesToNode.empty()) 
     {      
      std::pair<uint32_t, String> msg = firefighter.messagesToNode.front();
      if (!mesh.sendSingle(msg.first, msg.second)) {
        printToDisplay("Failed to send message: " + msg.second);
      }      
      firefighter.messagesToNode.pop();
    }    
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void informAllNodes(void *pvParameters) {
  while (1) {
    if (!firefighter.messagesToBroadcast.empty()) 
    {
      String msg = firefighter.messagesToBroadcast.front();

      for (auto node : mesh.getNodeList())
      {
        if (node != bridgeName)
        {
          if (!mesh.sendSingle(node, msg)) 
          {
          printToDisplay("Failed to send message: " + msg);
          }
        }   
      }
      firefighter.messagesToBroadcast.pop();
    } 
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }   
}

// This function is called when a node disconnects
void lostConnectionCallback(uint32_t nodeId) 
{
  Serial.printf("Lost Connection, nodeId = %u\n", nodeId);
  // Hitta noden i positionsList
  auto it = std::find_if(firefighter.positionsList.begin(), firefighter.positionsList.end(),
                          [nodeId](const std::pair<uint32_t, float>& entry) {
                              return entry.first == nodeId; // Matcha nodeId
                          });

  // Om noden hittades, ta bort den
  if (it != firefighter.positionsList.end()) 
  {
    firefighter.positionsList.erase(it);
    Serial.printf("Node %u removed from position list\n", nodeId);
  } 
  else 
  {
    Serial.printf("Node %u was not in the position list\n", nodeId);
  }
}


void meshUpdate(void *pvParameters) 
{
  while(1)
  {
    mesh.update();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
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
    mesh.sendSingle(leaderID, "No");
    setLEDOff();
  }

  if (helpButtonPressed) 
  {
    helpButtonPressed = false;
    printToDisplay("Help requested");
    firefighter.positionsList.clear();  // Rensa listan över positioner
    firefighter.messagesToBroadcast.push("ReqPos");  // Skicka förfrågan om position till alla noder  
  }

  if (yesButtonPressed) 
  {
    yesButtonPressed = false;
    printToDisplay("Yes pressed");
    mesh.sendSingle(leaderID, "Yes");
    firefighter.startMission(missionTargetRow, missionTargetColumn);
    firefighter.leaderID = leaderID;
    setLEDOff(); 
  }  
}
