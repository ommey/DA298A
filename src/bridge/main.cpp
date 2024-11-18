#include "namedMesh.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

String nodeName = "bridge"; // namnet på brygg-noden
namedMesh mesh; //variant på painlessMesh som kan skicka meddelanden till specifika noder





/*#include <Arduino.h>
#include "namedMesh.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

Scheduler userScheduler; 
namedMesh mesh;

String nodeName = "bridge"; // Unique name for the bridge node

void updateFromGUI();

Task taskUpdateFromGUI(TASK_MILLISECOND * 500, TASK_FOREVER, &updateFromGUI);


void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  //mesh.setDebugMsgTypes(ERROR | CONNECTION); 
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // Set the node name as "bridge"

  mesh.onReceive([](String &from, String &msg) {
  //  Serial.printf("Received message from %s: %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
////////////
auto nodes = mesh.getNodeList();
Serial.println("Connected nodes:");
for (auto node : nodes) {
  Serial.println(node);
}
///////////////
    Serial.printf("Connection table changed\n");
  });

  userScheduler.addTask(taskUpdateFromGUI);
  taskUpdateFromGUI.enable();
}

void updateFromGUI() {
    if (Serial.available()) {
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
}

void loop() {
  mesh.update();
}
*/