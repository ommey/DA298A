#include <painlessMesh.h>
#include <WiFi.h>

// Mesh network configuration
#define MESH_PREFIX "PainlessMeshNetwork"
#define MESH_PASSWORD "password123"
#define MESH_PORT 5555

Scheduler userScheduler;
painlessMesh mesh;

// Laptop server details
IPAddress serverIP(192,168,222,53); // Replace with your laptop's IP
uint16_t serverPort = 12345;          // Laptop server's port
WiFiClient client;

void sendMessageToLaptop() {
  if (client.connected()) {
    String message = "hey from esp " + String(mesh.getNodeId());
    client.print(message);
  }
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Message from %u: %s\n", from, msg.c_str());
  
  // Check if it's a relay message for the laptop
  if (msg.startsWith("laptop:")) {
    if (client.connected()) {
      client.print(msg.substring(7)); // Strip "laptop:" prefix
    }
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New connection, nodeId = %u\n", nodeId);
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin("YourWiFiSSID", "YourWiFiPassword"); // Replace with your WiFi credentials
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Connect to laptop's TCP server
  if (!client.connect(serverIP, serverPort)) {
    Serial.println("Failed to connect to laptop server!");
  }

  // PainlessMesh setup
  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION); // Set debugging
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);

  Task sendMessageTask(5000, TASK_FOREVER, &sendMessageToLaptop);

  sendMessageTask.enable();



 // userScheduler.addTask(Task(5000, TASK_FOREVER, &sendMessageToLaptop));
}

void loop() {
  mesh.update();
}
