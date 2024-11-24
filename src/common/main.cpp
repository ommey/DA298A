#include <Arduino.h>
#include "namedMesh.h"
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

using namespace std;

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

Firefighter firefighter;
String bridgeNAme = "bridge"; // namnet på brygga-noden
String nodeName; // namnet på noden
namedMesh mesh; //variant på painlessMesh som kan skicka meddelanden till specifika noder baserat på deras egenvalda namn.
std::map<String, std::pair<int, int>> contactList;  // Map of node IDs to their positions

void informBridge(void *pvParameters);  //dek av freertos task funktion som peeriodiskt uppdaterar gui med egenägd info
void meshUpdate(void *pvParameters);  //skit i denna, till för pinlessmesh,  freertos task funktion som uppdaterar meshen

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

void setup() 
{
  Serial.begin(115200);
  Serial.setTimeout(50);

  //mesh.setDebugMsgTypes(ERROR | CONNECTION);
  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);  // Starta meshen

  nodeName = String(mesh.getNodeId());  //namnet kan modifieras mes.getNodeId() är alltid unikt
  mesh.setName(nodeName);

  mesh.onReceive([](String &from, String &msg)
   {
    Serial.println(msg.c_str());
    
    if (from == bridgeNAme) {
      std::vector<std::string> tokens = tokenize(msg.c_str());
      if (tokens[0] == "Tick")
      {
        firefighter.Tick();
      }
      else if (tokens.size() == 3) 
      {
        if (tryParseInt(tokens[1]) && tryParseInt(tokens[2])) 
        {
          size_t row;
          size_t column;
          std::stoi(tokens[1], &row);
          std::stoi(tokens[2], &column);
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
        if (tryParseInt(tokens[2]) && tryParseInt(tokens[3])) {
          size_t row;
          size_t column;
          std::stoi(tokens[1], &row);
          std::stoi(tokens[2], &column);
          if (tokens[0] == "Firefighter" && firefighter.currentTile->getRow() == row && firefighter.currentTile->getColumn() == column) {
            firefighter.Die();
          } 
          else if (tokens[0] == "Victim")
          {
            firefighter.grid[row][column]->removeEvent(Event::VICTIM);
          }         
        }
      }
    }
    else if (from == "fireFighter") {
      //Mellan noderna kan jag inte er formattering
    }
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Connection table changed\n");
  });

//skapa tasks
xTaskCreate(meshUpdate, "meshUpdate", 10000, NULL, 1, NULL);
xTaskCreate(informBridge, "informBridge", 10000, NULL, 1, NULL); 

}

// This function is called when a new node connects
void newConnectionCallback(uint32_t nodeId) 
{
    Serial.printf("New Connection, nodeId = %u\n", nodeId);

    // Send this node's position to the new connection
    String posMsg = "Pos:" + String(firefighter.currentTile->getRow()) + "," + String(firefighter.currentTile->getColumn());
    mesh.sendSingle(nodeId, posMsg);
}

void informBridge(void *pvParameters) 
{
  while (1)
  {
     if (!firefighter.messagesToBridge.empty())
     {
      String msg = firefighter.messagesToBridge.front();
      Serial.println(msg);

      if (!mesh.sendSingle(bridgeNAme, msg)) 
      {
        Serial.println("Message send failed!");
      }
      firefighter.messagesToBridge.pop();
    }    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// This function is called when a node disconnects
void lostConnectionCallback(uint32_t nodeId) 
{
    Serial.printf("Lost Connection, nodeId = %u\n", nodeId);

    // Remove the disconnected node from the position map
    if (contactList.erase(String(nodeId))) 
    {
        Serial.printf("Node %u removed from position map\n", nodeId);
    } else 
    {
        Serial.printf("Node %u was not in the position map\n", nodeId);
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