#include <Arduino.h>
#include "namedMesh.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

String bridgeNAme = "bridge"; // namnet på brygga-noden
//
String nodeName; // namnet på noden
//
namedMesh mesh; //variant på painlessMesh som kan skicka meddelanden till specifika noder baserat på deras egenvalda namn.

void informBridge(void *pvParameters); //dek av freertos task funktion som peeriodiskt uppdaterar gui med egenägd info
void meshUpdate(void *pvParameters); //skit i denna, till för pinlessmesh,  freertos task funktion som uppdaterar meshen
void doFireFighterStuff(void *pvParameters); // freertos task funktion som gör branmansjobbet kontinueligt
void fireFighterStuff(); //själva brandmansjobbet, kallelse till denna stegar tillståndsmaskinen

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  //mesh.setDebugMsgTypes(ERROR | CONNECTION); 
  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT); // Starta meshen

  nodeName = String(mesh.getNodeId()); //namnet kan modifieras mes.getNodeId() är alltid unikt
  mesh.setName(nodeName); 

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message from %s: %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Connection table changed\n");
  });

//skapa tasks
  xTaskCreate(meshUpdate, "meshUpdate", 10000, NULL, 1, NULL);
  xTaskCreate(informBridge, "informBridge", 10000, NULL, 1, NULL); 
  xTaskCreate(doFireFighterStuff, "doFireFighterStuff", 10000, NULL, 1, NULL);
}

void loop() 
{
  // inget görs här, aktiviteter sköts i freeRTOS tasks
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

void meshUpdate(void *pvParameters){
    while(1) {
        mesh.update();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}