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
#include "hardware_config.h"
#include "painlessMesh.h"


using namespace std;

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

Firefighter firefighter;
uint32_t bridgeName = 1; // namnet på brygga-noden
String nodeName; // namnet på noden
painlessMesh mesh; //variant på painlessMesh som kan skicka meddelanden till specifika noder baserat på deras egenvalda namn.
uint32_t leaderID;  // ID of the node who sent help request
int missionTargetRow = 0;
int missionTargetColumn = 0;
int positionListCounter = 0;


void IRAM_ATTR handleButton1() {  //TODO: ändra vad knappen gör
  printToDisplay("Button 1 pressed\nHjälp");  // Test för att se att knapptryckning fungerar
  // TODO: Hjälpförfrågan sekvens
  firefighter.positionsList.clear();  // Rensa listan över positioner
  firefighter.messagesToBroadcast.push("ReqPos");
  // TODO: Sätt waiting state?
}
void IRAM_ATTR handleButton2() {// Yes-button
  printToDisplay("Button 2 pressed");  // Test för att se att knapptryckning fungerar
  // TODO: Handle button 2 press, when help is requested (maybe bool?), this button is yes.
  // I think we just want to toggle boolean here
  mesh.sendSingle(leaderID, "Yes");
  firefighter.startMission(missionTargetRow, missionTargetColumn);
  firefighter.leaderID = leaderID;
  
}
void IRAM_ATTR handleButton3() {// No-button
  printToDisplay("Button 3 pressed");  // Test för att se att knapptryckning fungerar
  mesh.sendSingle(leaderID, "No");
}

void informBridge(void *pvParameters);  //dek av freertos task funktion som peeriodiskt uppdaterar gui med egenägd info
void informSingleNode(void *pvParameters);
void meshUpdate(void *pvParameters);  //skit i denna, till för pinlessmesh,  freertos task funktion som uppdaterar meshen
void informAllNodes(void *pvParameters);

std::vector<std::string> tokenize(const std::string& expression) {
    std::vector<std::string> tokens;
    std::string token;

    for (char c : expression) {
        if (c == ' ') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

bool tryParseInt(const std::string& str) 
{
    try 
    {
        size_t pos;
        std::stoi(str, &pos);
        return pos == str.length();
    }
    catch (std::invalid_argument&) 
    {
        return false; // Strängen kunde inte tolkas som ett tal
    }
    catch (std::out_of_range&) 
    {
        return false; // Strängen representerar ett tal utanför int:s intervall
    }
}

void handlePositions(uint32_t from, std::vector<std::string> tokens)
{
  float dis = std::sqrt(std::pow(tryParseInt(tokens[1])-firefighter.targetTile->getRow(),2)+std::pow(tryParseInt(tokens[2])-firefighter.targetTile->getColumn(),2));
  firefighter.positionsList.push_back({from, dis}); // Spara nodens position i positionsList
  if (firefighter.positionsList.size() == mesh.getNodeList(false).size()-1) //Check if all nodes anwsered, if true, start sorting
  { 
    std::sort(firefighter.positionsList.begin(), firefighter.positionsList.end(),
    [](const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b) {
      return a.second < b.second; // Compare by distance
    });
    positionListCounter = 0;
    for (positionListCounter; positionListCounter < 3; positionListCounter++) {
      mesh.sendSingle(firefighter.positionsList[positionListCounter].first, "Help " + String(firefighter.targetTile->getRow()) + " " + String(firefighter.targetTile->getColumn()));
    }
  }
}

void handleHelpRequest(uint32_t from, std::vector<std::string> tokens)
{
  // TODO: spara id på avsändare.
  if (!firefighter.hasMission) 
  {
    leaderID = from;  // Spara id på avsändare
    setLEDColor(0, 0, 255);  // Blå testfärg
    printToDisplay("Help request recieved");
    missionTargetRow = std::stoi(tokens[1]);
    missionTargetColumn = std::stoi(tokens[2]);
  }
}

void setup() 
{
  Serial.begin(115200);
  Serial.setTimeout(50);

  // Init hardware, buttons and TFT display and LED
  hardwareInit();

  // Attach interrupts to the button pins
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), handleButton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2), handleButton2, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_3), handleButton3, FALLING);


  //mesh.setDebugMsgTypes(ERROR | CONNECTION);
  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);  // Starta meshen

  mesh.onReceive([](uint32_t from, String &msg) {
    Serial.println(msg.c_str());

    std::vector<std::string> tokens = tokenize(msg.c_str());

    if (from == bridgeName) {
      if (tokens[0] == "Tick") 
      {
        firefighter.Tick();
        printToDisplay("Tick\n");
      }
      else if (tokens.size() == 3) 
      {
        if (tryParseInt(tokens[1]) && tryParseInt(tokens[2])) 
        {
          size_t row = 0;
          size_t column = 0;
          row = std::stoi(tokens[1]);
          column = std::stoi(tokens[2]);
          
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
        }        
      } 
      else if (tokens.size() == 4 && tokens[1] == "dead")
      {
        if (tryParseInt(tokens[2]) && tryParseInt(tokens[3])) 
        {
          size_t row;
          size_t column;
          std::stoi(tokens[2], &row);
          std::stoi(tokens[3], &column);
          if (tokens[0] == "Firefighter" && firefighter.currentTile->getRow() == row && firefighter.currentTile->getColumn() == column) 
          {
            firefighter.Die();
          } 
          else if (tokens[0] == "Victim")
          {
            firefighter.grid[row][column]->removeEvent(Event::VICTIM);
          }         
        }
      }
    }
    else 
    {
      if (tokens[0] == "Pos") 
      {  
        handlePositions(from, tokens);
      }
      else if (tokens[0] == "ReqPos") 
      {
        mesh.sendSingle(from, "Pos " + String(firefighter.currentTile->getRow()) + " " + String(firefighter.currentTile->getColumn()));
      }
      else if (tokens[0] == "Help") 
      {
        handleHelpRequest(from, tokens);
      }
      else if (tokens[0] == "Yes") 
      {        
        firefighter.teamMembers.push_back(from);
      }
      else if (tokens[0] == "No") 
      {  
        mesh.sendSingle(firefighter.positionsList[positionListCounter].first, "Help " + String(firefighter.targetTile->getRow()) + " " + String(firefighter.targetTile->getColumn()));
        positionListCounter++;
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
    //Serial.printf("Connection table changed\n");
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
    //Serial.printf("New Connection, nodeId = %u\n", nodeId);

    // Send this node's Pussyition to the new connection
    String PussyMsg = "Pussy:" + String(firefighter.currentTile->getRow()) + "," + String(firefighter.currentTile->getColumn());
    mesh.sendSingle(nodeId, PussyMsg);
}

void informBridge(void *pvParameters) {
  while (1) {
     if (!firefighter.messagesToBridge.empty()) {
      String msg = firefighter.messagesToBridge.front();
      Serial.println(msg);

      if (!mesh.sendSingle(bridgeName, msg)) {
        //Serial.println("Message send failed!");
      }
      firefighter.messagesToBridge.pop();
    }    
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void informSingleNode(void *pvParameters) {
  while (1) {
     if (!firefighter.messagesToNode.empty()) {      
      std::pair<uint32_t, String> msg = firefighter.messagesToNode.front();
      mesh.sendSingle(msg.first, msg.second);
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
          mesh.sendSingle(node, msg);
        }   
      }
      firefighter.messagesToBroadcast.pop();
    } 
  }   
    vTaskDelay(500 / portTICK_PERIOD_MS);
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
void loop() {}
