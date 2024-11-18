#include <Arduino.h>
#include "namedMesh.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

Scheduler userScheduler;
namedMesh mesh;

String bridgeName = "bridge"; // Target node (bridge)
String nodeName; // Unique name for this node

//void broadCastCommon(String message);
//void sendToBridge();

Task taskSendMessage(TASK_SECOND * 10, TASK_FOREVER, []() {
    String msg = "Hello from " + nodeName;
    if (!mesh.sendSingle(bridgeName, msg)) {
        Serial.println("Message send failed!");
    }
});

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  //mesh.setDebugMsgTypes(ERROR | CONNECTION); 
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

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
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
  Serial.println(mesh.getName());
}

void loop() {
  mesh.update();
}
