#include <Arduino.h>
#include "namedMesh.h"
#include "Tile.h"
#include "Firefighter.h"
#include <array>
#include <iostream>
#include <random>
#include <vector>
#include <queue>
#include <cmath>
#include <unordered_map>
#include <string>
#include "hardware_config.h"

using namespace std;

enum class States {
    SEARCHING,
    TOTARGET,
    PUTTINGOUT,
    WAITING,
    CARRYING,
    PICKINGUPPERSON,
    PICKINGUPMATERIAL
} state;

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

#define SMOKE 0b0001
#define FIRE  0b0010
#define HAZMAT 0b0100
#define PERSON 0b1000

Firefighter FF;
int currX;
int currY;
int lastX;
int lastY;
int walls;
String msg;
std::random_device rd; 
std::mt19937 gen(rd()); 
std::uniform_int_distribution<> dis(1, 4);

String bridgeNAme = "bridge"; // namnet på brygga-noden
//
String nodeName; // namnet på noden
//
namedMesh mesh; //variant på painlessMesh som kan skicka meddelanden till specifika noder baserat på deras egenvalda namn.
std::map<String, std::pair<int, int>> contactList;  // Map of node IDs to their positions
queue<String> messagesToBridge;  //kö för meddelanden som ska skickas till bryggan | !push för att lägga till!

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

std::vector<std::string> tokenize(const std::string& expression) {
    std::vector<std::string> tokens;
    std::string token;

    for (char c : expression) {
        if (c == ' ') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

bool tryParseInt(const std::string& str) {
    try {
        size_t pos;
        std::stoi(str, &pos);

        // Kontrollera att hela strängen har parsats, t.ex. "123abc" skulle annars bli delvis parsad.
        return pos == str.length();
    }
    catch (std::invalid_argument&) {
        return false; // Strängen kunde inte tolkas som ett tal
    }
    catch (std::out_of_range&) {
        return false; // Strängen representerar ett tal utanför int:s intervall
    }
}

bool checkForEvent(int x, int y, int e, int w){
  bool up    = ((w & 1) == 0) && FF.getGrid()[x][y+1].getEvents() == e;
  bool right = ((w & (1 << 1)) == 0) && FF.getGrid()[x+1][y].getEvents() == e;
  bool down  = ((w & (1 << 2)) == 0) && FF.getGrid()[x][y-1].getEvents() == e;
  bool left  = ((w & (1 << 3)) == 0) && FF.getGrid()[x-1][y].getEvents() == e;

  return up || right || down || left;
}

void changeState(){
  if (FF.getHasTarget()) {
      Serial.printf("\n Goes to target");
      state = States::TOTARGET;
  } else if (checkForEvent(currX, currY, PERSON, walls)) {
      Serial.printf("\n Goes to picking up person");
      state = States::PICKINGUPPERSON; 
  } else if (FF.getGrid()[currX][currY].getEvents() == SMOKE || checkForEvent(currX, currY, SMOKE, walls) || checkForEvent(currX, currY, FIRE, walls)) {
      Serial.printf("\n Goes to putting out");
      state = States::PUTTINGOUT;
  } else if (checkForEvent(currX, currY, HAZMAT, walls)) {
      Serial.printf("\n Goes to picking up material");
      state = States::PICKINGUPMATERIAL;
  } else {
      Serial.printf("\n Goes to searching");
      state = States::SEARCHING;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  // Init hardware, buttons and TFT display
  hardwareInit();

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
    if (from == bridgeNAme) {
      std::vector<std::string> tokens = tokenize(msg.c_str());
      if (tokens.size() == 1 && tokens[0] == "Tick")
      {
        fireFighterStuff();
      }
      else if (tokens.size() == 3 && tokens[0] == "Fire") {
        if (tryParseInt(tokens[1]) && tryParseInt(tokens[2])) {
          size_t x;
          size_t y;
          std::stoi(tokens[1], &x);
          std::stoi(tokens[2], &y);
          FF.addEvent(x,y,FIRE);
        }
      }
      else if (tokens.size() == 3 && tokens[0] == "Smoke") {
        if (tryParseInt(tokens[1]) && tryParseInt(tokens[2])) {
          size_t x;
          size_t y;
          std::stoi(tokens[1], &x);
          std::stoi(tokens[2], &y);
          FF.addEvent(x,y,SMOKE);
        }
      }
      else if (tokens.size() == 3 && tokens[0] == "Victim")
      {
        if (tryParseInt(tokens[1]) && tryParseInt(tokens[2])) {
          size_t x;
          size_t y;
          std::stoi(tokens[1], &x);
          std::stoi(tokens[2], &y);
          FF.addEvent(x,y,PERSON);
        }
      }
      else if (tokens.size() == 3 && tokens[0] == "Hazmat")
      {
        if (tryParseInt(tokens[1]) && tryParseInt(tokens[2])) {
          size_t x;
          size_t y;
          std::stoi(tokens[1], &x);
          std::stoi(tokens[2], &y);
          FF.addEvent(x,y,HAZMAT);
        }
      }
      else if (tokens.size() == 4 && tokens[0] == "Firefighter" && tokens[1] == "dead")
      {
        //Om detta är din position DÖ x = tokens[2] y = tokens[3]
      }
      else if (tokens.size() == 4 && tokens[0] == "Victim" && tokens[1] == "dead")
      {
        if (tryParseInt(tokens[1]) && tryParseInt(tokens[2])) {
          size_t x;
          std::stoi(tokens[1], &x);
          size_t y;
          std::stoi(tokens[2], &y);
          FF.removeEvent(x,y,PERSON);
        }
      }
    }
    else if (from == "fireFighter") {
      //Mellan noderna kan jag inte er formattering
    }
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Connection table changed\n");
  });

//skapa tasks
  xTaskCreate(meshUpdate, "meshUpdate", 10000, NULL, 1, NULL);
  xTaskCreate(informBridge, "informBridge", 10000, NULL, 1, NULL); 
  //xTaskCreate(doFireFighterStuff, "doFireFighterStuff", 10000, NULL, 1, NULL);

  std::array<std::array<Tile, 6>, 8> grid; 
  for(int i = 0; i < 8; ++i){
      for (int j = 0; j < 6; ++j){
          grid[i][j] = Tile(i, j);
      }
  }
  grid[0][0].setWalls(0b1110);
  grid[1][0].setWalls(0b1101);
  grid[2][0].setWalls(0b0100);
  grid[3][0].setWalls(0b0111);
  grid[4][0].setWalls(0b1110);
  grid[5][0].setWalls(0b1110);
  grid[6][0].setWalls(0b1110);
  grid[7][0].setWalls(0b1110);
  grid[0][1].setWalls(0b1000);
  grid[1][1].setWalls(0b0110);
  grid[2][1].setWalls(0b1010);
  grid[3][1].setWalls(0b1110);
  grid[4][1].setWalls(0b1000);
  grid[7][1].setWalls(0b0011);
  grid[0][2].setWalls(0b1001);
  grid[1][2].setWalls(0b0001);
  grid[7][2].setWalls(0b0110);
  grid[0][3].setWalls(0b1100);
  grid[1][3].setWalls(0b0100);
  grid[2][3].setWalls(0b0010);
  grid[3][3].setWalls(0b1010);
  grid[4][3].setWalls(0b1000);
  grid[6][3].setWalls(0b0010);
  grid[7][3].setWalls(0b1011);
  grid[0][4].setWalls(0b1000);
  grid[6][4].setWalls(0b0010);
  grid[7][4].setWalls(0b1110);
  grid[0][5].setWalls(0b1001);
  grid[1][5].setWalls(0b0001);
  grid[2][5].setWalls(0b0011);
  grid[3][5].setWalls(0b1011);
  grid[4][5].setWalls(0b1001);
  grid[5][5].setWalls(0b0001);
  grid[6][5].setWalls(0b0001);
  grid[7][5].setWalls(0b0011);
  Tile startingTile(3,5);
  changeState();
  FF = Firefighter(1, startingTile, grid);
}

void handleSearching() {
    currX = FF.getCurrentTile().getX();
    currY = FF.getCurrentTile().getY();
    lastX = FF.getLastTile().getX();
    lastY = FF.getLastTile().getY();
    if (FF.getCurrentTile().getWalls() == 0b0111 || FF.getCurrentTile().getWalls() == 0b1011 || FF.getCurrentTile().getWalls() == 0b1101 || FF.getCurrentTile().getWalls() == 0b1110) {
        FF.move(lastX, lastY);
    } else {
        while (true) {
            int dir = dis(gen);
            if (dir == 1) {
                if ((FF.getGrid()[currX][currY].getWalls() & 1) == 0 && currY != 5 && lastY != currY+1) {
                    FF.move(currX, currY+1);
                    break;
                } 
            } else if (dir == 2) {
                if ((FF.getGrid()[currX][currY].getWalls() & (1 << 1)) == 0 && currX != 7 && lastX != currX+1) {
                    FF.move(currX+1, currY);
                    break;
                }
            } else if (dir == 3) {
                if ((FF.getGrid()[currX][currY].getWalls() & (1 << 2)) == 0 && currY != 0 && lastY != currY-1) {
                    FF.move(currX, currY-1);
                    break;
                }
            } else {
                if ((FF.getGrid()[currX][currY].getWalls() & (1 << 3)) == 0 && currX != 0 && lastX != currX-1) {
                    FF.move(currX-1, currY);
                    break;
                }
            }
        }
    }

    walls = FF.getCurrentTile().getWalls();
    lastX = FF.getLastTile().getX();
    lastY = FF.getLastTile().getY();
    currX = FF.getCurrentTile().getX();
    currY = FF.getCurrentTile().getY();

    Serial.printf("\n Moved to %d, %d", currX, currY);

    msg = "Firefighter from " + String(lastX) + " " + String(lastY) + " to " + String(currX) + " " + String(currY);

    messagesToBridge.push(msg); // Skickar senast och nuvarande koordinater till bridge

    changeState();
}

void handleToTarget() {
    currX = FF.getCurrentTile().getX();
    currY = FF.getCurrentTile().getY();
    int targetX = FF.getTargetTile().getX();
    int targetY = FF.getTargetTile().getY();

    if(currX != targetX) {
        if(currX > targetX){        // Vi är till höger om target tile
            FF.move(currX-1, currY);
        } else{                     // Vi är till vänster om target tile    
            FF.move(currX+1, currY);
        }
    } else if(currY != targetY) {
        if(currY > targetY){        // Vi är över target tile
            FF.move(currX, currY-1);
        } else{                     // Vi är under target tile    
            FF.move(currX, currY+1);
        }                    
    } else {                        // Vi är på target tile
        // if all firefighters are here
        // start carrying
        // else wait
        FF.setHasTarget(false);
        state = States::SEARCHING;
    }

}

void handlePuttingOut() {

    currX = FF.getCurrentTile().getX();
    currY = FF.getCurrentTile().getY();
    int putOutX;
    int putOutY;
    int event;

    if (FF.getGrid()[currX][currY].getEvents() != 0){
        putOutX = currX;
        putOutY = currY;
        event = FF.handleSmokeFire(putOutX, putOutY);
    }
    else if(FF.getGrid()[currX][currY+1].getEvents() != 0){
        putOutX = currX;
        putOutY = currY+1;
        event = FF.handleSmokeFire(putOutX, putOutY);
    }
    else if(FF.getGrid()[currX+1][currY].getEvents() != 0){
        putOutX = currX+1;
        putOutY = currY; 
        event = FF.handleSmokeFire(putOutX, putOutY);              
    }
    else if(FF.getGrid()[currX][currY-1].getEvents() != 0){
        putOutX = currX;
        putOutY = currY-1;  
        event = FF.handleSmokeFire(putOutX, putOutY);              
    }
    else if(FF.getGrid()[currX-1][currY].getEvents() != 0){
        putOutX = currX-1;
        putOutY = currY; 
        event = FF.handleSmokeFire(putOutX, putOutY);               
    }

    walls = FF.getCurrentTile().getWalls();
    currX = FF.getCurrentTile().getX();
    currY = FF.getCurrentTile().getY();

    if (event == SMOKE) {
        msg = "Smoke putout " + String(putOutX) + " " + String(putOutY);
    } else {
        msg = "Fire putout " + String(putOutX) + " " + String(putOutY);
    }

    messagesToBridge.push(msg); // Skickar event och koordinater till bridge

    // Skicka event och koordinater till brandmän

    changeState();

}

void handleWaiting() {
    // if all firefighters are here
    // send start message and start carrying
    // else continue waiting
    state = States::CARRYING;
}

void handleCarrying() {
    currX = FF.getCurrentTile().getX();
    currY = FF.getCurrentTile().getY();
    int e = FF.getGrid()[currX][currY].getEvents();
    
    if(currY >= 4) {                // Vi är på översta sidan av planen, ska gå uppåt
        if(currY == 7){             // Kolla ifall vi är vid översta gränsen av planen
            if (FF.getCarryingPerson()) {

                FF.removeEvent(currX,currY,PERSON);
                msg = "Victim saved " + String(currX) + " " + String(currY);
                messagesToBridge.push(msg);
                FF.setCarryingPerson(false);

            } else if (FF.getCarryingHazmat()) {

                FF.removeEvent(currX,currY,HAZMAT);
                msg = "Hazmat saved " + String(currX) + " " + String(currY);
                messagesToBridge.push(msg);
                FF.setCarryingHazmat(false);
            }   
            changeState();
        } else {
            int nextE = FF.getGrid()[currX][currY+1].getEvents();
            if (FF.getCarryingPerson()) {

                FF.removeEvent(currX, currY, PERSON);
                FF.addEvent(currX, currY+1, PERSON);
                msg = "Victim from " + String(currX) + " " + String(currY) + " " + String(currX) + String(currY+1);
                messagesToBridge.push(msg);

                msg = "Firefighter from " + String(currX) + " " + String(currY) + " " + String(currX) + String(currY+1);
                messagesToBridge.push(msg);
            } else if(FF.getCarryingHazmat()){

                FF.removeEvent(currX, currY, HAZMAT);
                FF.addEvent(currX, currY+1, HAZMAT);
                msg = "Hazmat from " + String(currX) + " " + String(currY) + " " + String(currX) + String(currY+1);
                messagesToBridge.push(msg);

                msg = "Firefighter from " + String(currX) + " " + String(currY) + " " + String(currX) + String(currY+1);
                messagesToBridge.push(msg);
            }                        
            FF.move(currX, currY+1);
        }
    } else {                        
        if(currY == 0){             // Kolla ifall vi är vid nedersta gränsen av planen
            if (FF.getCarryingPerson()) {

                FF.removeEvent(currX, currY, PERSON);
                msg = "Victim saved " + String(currX) + " " + String(currY);
                messagesToBridge.push(msg);
                FF.setCarryingPerson(false);

            } else if(FF.getCarryingHazmat()){

                FF.removeEvent(currX,currY,HAZMAT);
                msg = "Hazmat saved " + String(currX) + " " + String(currY);
                messagesToBridge.push(msg);
                FF.setCarryingHazmat(false);

            }   
            changeState ();

        } else {
            int nextE = FF.getGrid()[currX][currY-1].getEvents();
            if (FF.getCarryingPerson()) {

                FF.removeEvent(currX, currY, PERSON);
                FF.addEvent(currX, currY-1, PERSON);
                msg = "Victim from " + String(currX) + " " + String(currY) + " " + String(currX) + String(currY-1);
                messagesToBridge.push(msg);

                msg = "Firefighter from " + String(currX) + " " + String(currY) + " " + String(currX) + String(currY-1);
                messagesToBridge.push(msg);

            } else if(FF.getCarryingHazmat()){
                FF.removeEvent(currX, currY, HAZMAT);
                FF.addEvent(currX, currY-1, HAZMAT);
                msg = "Hazmat from " + String(currX) + " " + String(currY) + " " + String(currX) + String(currY-1);
                messagesToBridge.push(msg);

                msg = "Firefighter from " + String(currX) + " " + String(currY) + " " + String(currX) + String(currY-1);
                messagesToBridge.push(msg);
            }                
            FF.move(currX, currY-1);
            int currEvent = FF.getGrid()[currX][currY-1].getEvents();
            Serial.printf("\n Event of current tile: %d", currEvent);
        }
    }

// broadcasta till brandmän var saker och ting flyttas

}

void handlePickingUpPerson() {
    currX = FF.getCurrentTile().getX();
    currY = FF.getCurrentTile().getY();

    if(FF.getGrid()[currX][currY+1].getEvents() == PERSON){         // Person är norr
        FF.move(currX,currY+1);
        Serial.printf("\n Gått upp ett steg");
    } else if(FF.getGrid()[currX+1][currY].getEvents() == PERSON){  // Person är öst
        FF.move(currX+1,currY);
        Serial.printf("\n Gått höger ett steg");
    } else if(FF.getGrid()[currX][currY-1].getEvents() == PERSON){  // Person är söder
        FF.move(currX,currY-1);
        Serial.printf("\n Gått ner ett steg");
    } else if(FF.getGrid()[currX-1][currY].getEvents() == PERSON){  // Person är väst
        FF.move(currX-1,currY);
        Serial.printf("\n Gått vänster ett steg");
    }

    FF.setCarryingPerson(true);

    lastX = FF.getLastTile().getX();
    lastY = FF.getLastTile().getY();
    currX = FF.getCurrentTile().getX();
    currY = FF.getCurrentTile().getY();

    msg = "Firefighter from " + String(lastX) + " " + String(lastY) + " to " + String(currX) + " " + String(currY);

    messagesToBridge.push(msg); // Skickar senast och nuvarande koordinater till bridge

    state = States::WAITING;
}

void handlePickingUpMaterial() {
    currX = FF.getCurrentTile().getX();
    currY = FF.getCurrentTile().getY();

    if(FF.getGrid()[currX][currY+1].getEvents() == HAZMAT){         // Materialet är norr
        FF.move(currX,currY+1);
    } else if(FF.getGrid()[currX+1][currY].getEvents() == HAZMAT){  // Materialet är öst
        FF.move(currX+1,currY);
    } else if(FF.getGrid()[currX][currY-1].getEvents() == HAZMAT){  // Materialet är söder
        FF.move(currX,currY-1);
    } else if(FF.getGrid()[currX+1][currY].getEvents() == HAZMAT){  // Materialet är väst
        FF.move(currX-1,currY);
    }

    FF.setCarryingHazmat(true);

    lastX = FF.getLastTile().getX();
    lastY = FF.getLastTile().getY();
    currX = FF.getCurrentTile().getX();
    currY = FF.getCurrentTile().getY();

    msg = "Firefighter from " + String(lastX) + " " + String(lastY) + " to " + String(currX) + " " + String(currY);

    messagesToBridge.push(msg); // Skickar senast och nuvarande koordinater till bridge

    state = States::CARRYING;
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

void fireFighterStuff() {
    Serial.printf("\n Coordinates of firefighter %d: %d, %d", FF.getId(), FF.getCurrentTile().getX(), FF.getCurrentTile().getY());

    switch(state){
        case States::SEARCHING:

            Serial.printf("\n Searching"); 

            handleSearching();

            break;

        case States::TOTARGET:

            Serial.printf("\n To target");

            handleToTarget();

            break;

        case States::PUTTINGOUT:

            Serial.printf("\n Putting out");

            handlePuttingOut();

            break;

        case States::WAITING:

            Serial.printf("\n Waiting");

            handleWaiting();

            break;

        case States::CARRYING:

            Serial.printf("\n Carrying");

            handleCarrying();

            break;

        case States::PICKINGUPPERSON:

            Serial.printf("\n Picking up person");

            handlePickingUpPerson();

            break;

        case States::PICKINGUPMATERIAL:

            Serial.printf("\n Picking up material");

            handlePickingUpMaterial();

            break;
    };
}
/*
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
*/

void informBridge(void *pvParameters) {
  while (1)
  {
    if (!messagesToBridge.empty()) {
      String msg = messagesToBridge.front();
      if (!mesh.sendSingle(bridgeNAme, msg)) {
        Serial.println("Message send failed!");
      }
      messagesToBridge.pop();
    }
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// This function is called when a new node connects
void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("New Connection, nodeId = %u\n", nodeId);

    // Send this node's position to the new connection
    String posMsg = "Pos:" + String(FF.getCurrentTile().getX()) + "," + String(FF.getCurrentTile().getY());
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

void meshUpdate(void *pvParameters) {
    while(1) {
        mesh.update();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}