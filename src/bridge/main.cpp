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
            int index = msg.indexOf(':');
            if (index > 0) { 
                String receiver_node = msg.substring(0, index);
                String message = msg.substring(index + 1);
                if (receiver_node == "all") {
                    if (mesh.sendBroadcast(message)) {
                        Serial.println("Message sent to all");
                    } else {
                        Serial.println("Failed to send broadcast");
                    }
                } else {
                    if (mesh.sendSingle(receiver_node, message)) {
                        Serial.println("Message sent to " + receiver_node);
                    } else {
                        Serial.println("Failed to send to " + receiver_node);
                    }
                }
            } else {
                Serial.println("Invalid message format. Use 'nodeName:message'.");
            }
        } 
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void meshUpdate(void *pvParameters){
    while(1) {
        mesh.update();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void updateParticipantsStatus(void *pvParameters) {
    while (1) {
        Serial.print("participants:");
        auto nodes = mesh.getNodeList();
        for (auto iter = nodes.begin(); iter != nodes.end(); ++iter) {
            Serial.print(*iter);
            if (std::next(iter) != nodes.end()) {
                Serial.print(":");
            }
        }
        Serial.println();
        vTaskDelay(100 / portTICK_PERIOD_MS); // Adjust the delay as needed
    }
}

void printTable() {
    auto nodes = mesh.getNodeList();
Serial.println("Connected nodes:");
for (auto node : nodes) {
  Serial.println(node);
}
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  //mesh.setDebugMsgTypes(ERROR | CONNECTION); 

  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT); // Starta meshen

  mesh.setName(nodeName);

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message from %s: %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    printTable();
  });

    xTaskCreate(meshUpdate, "meshUpdate", 10000, NULL, 1, NULL); // Skapa en task som uppdaterar meshen
    xTaskCreate(updateFromGUI, "updateFromGUI", 10000, NULL, 1, NULL); // Skapa en task som lyssnar på Serial
    xTaskCreate(updateParticipantsStatus, "updateParticipantsStatus", 10000, NULL, 1, NULL); // Skapa en task som skickar fake-meddelanden
}

void loop() 
{
    // inget görs här, aktiviteter sköts i freeRTOS tasks
}