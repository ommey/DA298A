#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "namedMesh.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

TFT_eSPI tft = TFT_eSPI();

String nodeName = "bridge"; // namnet på brygg-noden
namedMesh mesh; //variant på painlessMesh som kan skicka meddelanden till specifika noder

void printToDisplay(String message);

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
                        printToDisplay("Message sent to all");
                    } else {
                        Serial.println("Failed to send broadcast");
                        printToDisplay("Failed to send broadcast");
                    }
                } else {
                    if (mesh.sendSingle(receiver_node, message)) {
                        Serial.println("Message sent to " + receiver_node);
                        printToDisplay("Message sent to " + receiver_node);
                    } else {
                        Serial.println("Failed to send to " + receiver_node);
                        printToDisplay("Failed to send to " + receiver_node);
                    }
                }
            } else {
                Serial.println("Invalid message format. Use 'nodeName:message'.");
            }
        } 
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void printToDisplay(String message) {
    tft.fillScreen(TFT_BLUE);
    tft.setCursor(10, 10);
    tft.print(message);
}

void meshUpdate(void *pvParameters){
    while(1) {
        mesh.update();
        vTaskDelay(50 / portTICK_PERIOD_MS);
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
  //Serial.setTimeout(50);  // Behövs inte???

  //mesh.setDebugMsgTypes(ERROR | CONNECTION); 

  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT); // Starta meshen

  mesh.setName(nodeName);

  mesh.onReceive([](String &from, String &msg) {
    Serial.println(msg.c_str());
    printToDisplay(msg);
  });

  mesh.onChangedConnections([]() {
    //printTable();
  });

    xTaskCreate(meshUpdate, "meshUpdate", 10000, NULL, 1, NULL); // Skapa en task som uppdaterar meshen
    xTaskCreate(updateFromGUI, "updateFromGUI", 10000, NULL, 1, NULL); // Skapa en task som lyssnar på Serial

    // Init display
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
}

void loop() {}  // inget görs här, aktiviteter sköts i freeRTOS tasks
