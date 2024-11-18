#include <Arduino.h>
#include "namedMesh.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

String bridgeNAme = "bridge"; // namnet på brygga-noden
//
String nodeName; // namnet på noden
//
namedMesh mesh; //variant på painlessMesh som kan skicka meddelanden till specifika noder

void informBridge(){
  String msg = "Hello from " + nodeName;

      if (!mesh.sendSingle(bridgeNAme, msg)) {
        Serial.println("Message send failed!");
      }  
}

void informBridge(void *pvParameters) {
    while (1)
    {
      informBridge();
      vTaskDelay(1000 / portTICK_PERIOD_MS);    
    }
    
    
}

void meshUpdate(void *pvParameters){
    while(1) {
        mesh.update();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  //mesh.setDebugMsgTypes(ERROR | CONNECTION); 
  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);

  // Assign a unique name based on the node's ID
  nodeName = String(mesh.getNodeId());
  mesh.setName(nodeName); 

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message from %s: %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Connection table changed\n");
  });

  // Add and enable the task for sending messages
  xTaskCreate(meshUpdate, "meshUpdate", 10000, NULL, 1, NULL);
  xTaskCreate(informBridge, "informBridge", 10000, NULL, 1, NULL);
}

void loop() 
{
  // inget görs här, aktiviteter sköts i freeRTOS tasks
}