#include <Arduino.h>
#include "namedMesh.h"
#include "Firefighter.h"
#include "hardware_config.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

std::map<String, std::pair<int, int>> contactList;  // Map of node IDs to their positions
Firefighter hoesHolder;  // This firefighter
String bridgeNAme = "bridge";  // namnet på brygga-noden
String nodeName;  // namnet på noden
namedMesh mesh;  //variant på painlessMesh som kan skicka meddelanden till specifika noder baserat på deras egenvalda namn.

volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;

void IRAM_ATTR handleButton1() { button1Pressed = true; }
void IRAM_ATTR handleButton2() { button2Pressed = true; }
void IRAM_ATTR handleButton3() { button3Pressed = true; }

void informBridge(void *pvParameters);  //dek av freertos task funktion som peeriodiskt uppdaterar gui med egenägd info
void meshUpdate(void *pvParameters);  //skit i denna, till för pinlessmesh,  freertos task funktion som uppdaterar meshen
void doFireFighterStuff(void *pvParameters);  // freertos task funktion som gör branmansjobbet kontinueligt
void fireFighterStuff();  //själva brandmansjobbet, kallelse till denna stegar tillståndsmaskinen

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);
  hoesHolder = Firefighter();  //skapa en brandman

  // Init hardware, buttons and TFT display
  if(hardwareInit()) {
    Serial.println("Hardware init success");
  } else {
    Serial.println("Hardware init failed");
  }

  // Attach interrupts to the button pins
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), handleButton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_2), handleButton2, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_3), handleButton3, FALLING);


  //mesh.setDebugMsgTypes(ERROR | CONNECTION);
  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);  // Starta meshen

  nodeName = String(mesh.getNodeId());  //namnet kan modifieras mes.getNodeId() är alltid unikt
  mesh.setName(nodeName);

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message from %s: %s\n", from.c_str(), msg.c_str());

    if (msg.startsWith("Pos:")) {
        int commaIdx = msg.indexOf(',');
        int x = msg.substring(4, commaIdx).toInt();
        int y = msg.substring(commaIdx + 1).toInt();
        contactList[from] = std::make_pair(x, y);
        Serial.println("Updated contact list:");
        for (const auto& contact : contactList) {
          Serial.printf("Node: %s, Position: (%d, %d)\n", contact.first.c_str(), contact.second.first, contact.second.second);
        }
    }

  });

  mesh.onChangedConnections([]() {
    Serial.printf("Connection table changed\n");
  });

//skapa tasks
  xTaskCreate(meshUpdate, "meshUpdate", 10000, NULL, 1, NULL);
  xTaskCreate(informBridge, "informBridge", 10000, NULL, 1, NULL);
  //xTaskCreate(doFireFighterStuff, "doFireFighterStuff", 10000, NULL, 1, NULL);
}

// inget görs här, aktiviteter sköts i freeRTOS tasks
void loop() {
  // Check if buttons were pressed and handle accordingly
  if (button1Pressed) {
    button1Pressed = false;
    Serial.println("Button 1 pressed");
    // Handle button 1 press
  }
  if (button2Pressed) {
    button2Pressed = false;
    Serial.println("Button 2 pressed");
    // Handle button 2 press
  }
  if (button3Pressed) {
    button3Pressed = false;
    Serial.println("Button 3 pressed");
    // Handle button 3 press
  }
}

void fireFighterStuff(){
  Serial.print("Branmannen inom mig jobbar hårt "); //här går tillståndsmaskinen in istället för denna printout
}

void doFireFighterStuff(void *pvParameters){
  int work = 0; //räknare som kan användas för att testa tick-funktionen
  while(1){

    fireFighterStuff();

    //********räknare som kan användas för att testa tick-funktionen
    work++;
    if (work > 1000)
    {
      work = 0;
    }
    Serial.println(work); 
    //*********räknare som kan användas för att testa tick-funktionen

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void informBridge(void *pvParameters) {
  while (1)
  {
    String msg = "Hello from " + nodeName;
    if (!mesh.sendSingle(bridgeNAme, msg)) {
      Serial.println("Message send failed!");
    }  
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// This function is called when a new node connects
void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("New Connection, nodeId = %u\n", nodeId);

    // Send this node's position to the new connection
    String posMsg = "Pos:" + String(hoesHolder.getCurrentTile().getX()) + "," + String(hoesHolder.getCurrentTile().getY());
    mesh.sendSingle(nodeId, posMsg);
}

// This function is called when a node disconnects
void lostConnectionCallback(uint32_t nodeId) {
    Serial.printf("Lost Connection, nodeId = %u\n", nodeId);

    // Remove the disconnected node from the position map
    if (contactList.erase(String(nodeId))) {
        Serial.printf("Node %u removed from position map\n", nodeId);
    } else {
        Serial.printf("Node %u was not in the position map\n", nodeId);
    }
}

void meshUpdate(void *pvParameters){
    while(1) {
        mesh.update();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}