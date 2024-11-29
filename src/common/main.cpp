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



void setup() 
{
  Serial.begin(115200);
  Serial.setTimeout(50);


  //mesh.setDebugMsgTypes(ERROR | CONNECTION);
  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);  // Starta meshen


  nodeName = String(mesh.getNodeId());  //namnet kan modifieras mes.getNodeId() är alltid unikt


  mesh.setName(nodeName);

  mesh.onReceive([](String &from, String &msg) {
    Serial.println(msg.c_str());


    });

  mesh.onChangedConnections([]() {
  });

  //skapa tasks
  xTaskCreate(meshUpdate, "meshUpdate", 10000, NULL, 1, NULL);
  xTaskCreate(informBridge, "informBridge", 10000, NULL, 1, NULL); 

}


void informBridge(void *pvParameters) {
  while (1) {
     if (!firefighter.messagesToBridge.empty()) {
      String msg = firefighter.messagesToBridge.front();
      Serial.println(msg);

      if (!mesh.sendSingle(bridgeNAme, msg)) {
        //Serial.println("Message send failed!");
      }
      firefighter.messagesToBridge.pop();
    }    
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
/*
// Task that sends a message to the bridge every second
QueueHandle_t xQueue;  // Flytta upp till globala variabler
// Create a queue capable of containing 10 strings
xQueue = xQueueCreate(10, sizeof(String));
if (xQueue == NULL) {
  Serial.println("Failed to create queue");
}  // Flytta upp till setup
xTaskCreate(sendMessagesTask, "sendMessagesTask", 10000, NULL, 1, NULL);  // Flytta upp till setup
void sendMessagesTask(void *pvParameters) {
  while (1) {
    if (xQueueReceive(xQueue, &msg, portMAX_DELAY) == pdPASS) {
      if (!mesh.sendSingle(bridgeNAme, msg)) {
        Serial.println("Message send failed!");
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // TODO: Kan vara onödgt att ha delay på tasks ! Undersöker
  }
}
*/
// This function is called when a node disconnects

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
