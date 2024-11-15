#include <Arduino.h>
#include "namedMesh.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

Scheduler userScheduler; 
namedMesh mesh;

String nodeName = "bridge"; // Unique name for the bridge node

void setup() {
  Serial.begin(115200);

  //mesh.setDebugMsgTypes(ERROR | CONNECTION); 
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // Set the node name as "bridge"

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message from %s: %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Connection table changed\n");
  });
}

void loop() {
  mesh.update();
}
