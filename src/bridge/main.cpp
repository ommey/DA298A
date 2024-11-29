#include <Arduino.h>
#include "namedMesh.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

String nodeName = "bridge"; // namnet på brygg-noden
namedMesh mesh; //variant på painlessMesh som kan skicka meddelanden till specifika noder

void updateFromGUI(void *pvParameters){
    while(1) {
        if (Serial.available()>0){  
            String msg = Serial.readStringUntil('\n');
                    if (mesh.sendBroadcast(msg)) {
                        //Serial.println("Message sent to all");
                    } else {
                        //Serial.println("Failed to send broadcast");
                    }      
        } 
        vTaskDelay(100 / portTICK_PERIOD_MS);
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
  Serial.setTimeout(50);  // Behövs inte???

  //mesh.setDebugMsgTypes(ERROR | CONNECTION); 

  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT); // Starta meshen

  mesh.setName(nodeName);

  mesh.onReceive([](String &from, String &msg) {
    Serial.println(msg.c_str());
  });

  mesh.onChangedConnections([]() {
    //printTable();
  });

    xTaskCreate(meshUpdate, "meshUpdate", 10000, NULL, 1, NULL); // Skapa en task som uppdaterar meshen
    xTaskCreate(updateFromGUI, "updateFromGUI", 10000, NULL, 1, NULL); // Skapa en task som lyssnar på Serial
}

void loop() {}  // inget görs här, aktiviteter sköts i freeRTOS tasks
