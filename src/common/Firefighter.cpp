#include "Firefighter.h"

Firefighter::Firefighter() 
    : gen(esp_random()), 
      dist(1, 4),
      missionTargetRow(0),
      missionTargetColumn(0),
      positionListCounter(0),
      tickCounter(0),
      hasMission(false),
      nbrFirefighters(1),
      pendingHelp(false),
      teamArrived(false),
      state(State::SEARCHING)
{
    // Lägg till debugutskrifter
    Serial.println("Firefighter constructor: Initializing...");

    // Starta task
    if (xTaskCreate(&Firefighter::messageHandlerTask, "MessageHandler", 4096, this, 1, NULL) != pdPASS) {
        Serial.println("Misslyckades med att skapa MessageHandler-task");
    }
}



void Firefighter::messageHandlerTask(void *pvParameters) 
{
    Firefighter* self = static_cast<Firefighter*>(pvParameters);  // Få tillgång till instansen
    Message message;
    while (1)
    {
        if (self->comms.incomingMessages == nullptr) {
            Serial.println("Error: incomingMessages queue is null");
            vTaskDelete(NULL);
        }

        if (xQueueReceive(self->comms.incomingMessages, &message, portMAX_DELAY)) 
        {
            self->handleMessage(message.to, message.msg);  // Hantera meddelandet
        }
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}

void Firefighter::handleMessage(uint32_t from, const char* msg) 
{
    String message(msg);

    printToDisplay("Recieved: " + message);

    if (msg == "Tick") 
    { 
        Tick(); 
    }
    else if (msg == "ReqPos") 
    {
      comms.meshPush("Pos " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()), from);
    }
    else if (msg == "Yes") 
    { 
      printToDisplay("Yes recieved");       
      teamMembers.push_back(from);
    }
    else if (msg == "No") 
    { 
      printToDisplay("No recieved");
      for (int i = 0; i < teamMembers.size(); i++) {
          if (positionsList[positionListCounter].first == teamMembers[i]) {
            i = 0;
            positionListCounter = (positionListCounter + 1) % positionsList.size();
          }
      }
      comms.meshPush("Help " + String(grid.targetTile->getRow()) + " " + String(grid.targetTile->getColumn()), positionsList[positionListCounter].first);
      positionListCounter = (positionListCounter + 1) % positionsList.size();
    }
    else if (msg == "Arrived")
    {
      nbrFirefighters++;
    }
    else if (msg == "TeamArrived")
    {
      TeamArrived();
    } 
    else 
    {
        int row = 0;
        int column = 0;
        std::vector<String> tokens = tokenize(msg);   

        if (tokens.size() == 3 && tryParseInt(tokens[1], row) && tryParseInt(tokens[2], column)) 
        {  
            if (tokens[0] == "MaybeDie")
            {
                Die(row, column);
            }           
            else if (tokens[0] == "Pos")
            {
                handlePositions(from, row, column);
            }
            else if (tokens[0] == "Help") 
            {
                handleHelpRequest(from, row, column);      
            }
            else 
            {
                grid.update(tokens[0], row, column);
            }
        }
    }
}

std::vector<String> Firefighter::tokenize(const String& expression) 
{
    std::vector<String> tokens;
    String token;

    for (int i = 0; i < expression.length(); ++i) {
        char c = expression[i];
        if (c == ' ') {
            if (!token.isEmpty()) {
                tokens.push_back(token);
                token = ""; 
            }
        } else {
            token += c;
        }
    }
    if (!token.isEmpty()) {
        tokens.push_back(token);
    }
    return tokens;
}

bool Firefighter::tryParseInt(const String& str, int& outValue) 
{
    char* endPtr;
    long value = strtol(str.c_str(), &endPtr, 10); // Försök att konvertera strängen

    if (*endPtr == '\0') { // Kontrollera att hela strängen är ett giltigt tal
        outValue = static_cast<int>(value);

        // Kontrollera att värdet ligger inom intervallet för int
        if (value >= INT_MIN && value <= INT_MAX) {
            return true;
        }
    }
    return false; // Parsning misslyckades
}

void Firefighter::handlePositions(uint32_t from, int row, int column)
{  
  float dis = std::sqrt(std::pow(row-grid.targetTile->getRow(),2)+std::pow(column-grid.targetTile->getColumn(),2));
  positionsList.push_back({from, dis}); // Spara nodens position i positionsList
  if (positionsList.size() == comms.getNodeList().size()-3) //Check if all nodes anwsered, if true, start sorting
  { 
    std::sort(positionsList.begin(), positionsList.end(),
    [](const std::pair<uint32_t, float>& a, const std::pair<uint32_t, float>& b) 
    {
      return a.second < b.second; // Compare by distance
    });
    
    positionListCounter = 0;
    
    for (positionListCounter; positionListCounter < 1; positionListCounter++) 
    {
      printToDisplay("Called firefighter: " + String(positionsList[positionListCounter].first) + " with distance: " + String(positionsList[positionListCounter].second));
      comms.meshPush("Help " + String(grid.targetTile->getRow()) + " " + String(grid.targetTile->getColumn()), positionsList[positionListCounter].first);
    }
  }
}

void Firefighter::handleHelpRequest(uint32_t from, int row, int column)
{
  // TODO: spara id på avsändare.
  leaderID = from;  // Spara id på avsändare
  setLEDColor(0, 0, 255);  // Blå hjälpfärg
  printToDisplay("Help request recieved");
  missionTargetRow = row;
  missionTargetColumn = column;
  tickCounter = 0;
  pendingHelp = true;
} 

void Firefighter::move(const Tile* destination)
{  
    grid.lastTile = grid.currentTile;
    grid.currentTile = grid.getTile(destination->getRow(), destination->getColumn());
    String msg = "Firefighter from " + String(grid.lastTile->getRow()) + " " + String(grid.lastTile->getColumn()) + " to " + grid.currentTile->getRow() + " " + grid.currentTile->getColumn();
    comms.meshPush(msg, BRIDGE_NAME); 
}

void Firefighter::changeState()
{
    if (hasMission) 
    {
      //Serial.printf("Goes to target\n");
      //printToDisplay("Goes to target");
      state = State::MOVING_TO_TARGET;
    }
    else if (grid.checkForEvent(Event::VICTIM))
    {
      //Serial.printf("Goes to picking up person\n");
      //printToDisplay("Goes to picking up person");
      String msg = "RemoveVictim " + String(grid.targetTile->getRow()) + " " + String(grid.targetTile->getColumn());
      comms.meshPush(msg, 0); // To broadcast
      state = State::MOVING_TO_TARGET; 
    }
    else if (grid.checkForEvent(Event::FIRE)) 
    {
      //Serial.printf("Goes to putting out fire\n");
      //printToDisplay("Goes to putting out fire");
      state = State::PUTTING_OUT_FIRE;
    } 
    else if (grid.checkForEvent(Event::SMOKE))
    {
      //Serial.printf("Goes to putting out smoke\n");
      //printToDisplay("Goes to putting out smoke");
      state = State::PUTTING_OUT_SMOKE;
    } 
    else if (grid.checkForEvent(Event::HAZMAT))
    {
      //Serial.printf("Goes to picking up material\n");
      //printToDisplay("Goes to picking up material");
      state = State::MOVING_HAZMAT;
    } 
    else 
    {
      //Serial.printf("Goes to searching\n");
      //printToDisplay("Goes to searching");
      state = State::SEARCHING;
    }
}


void Firefighter::searchForTarget()
{
    if (grid.atDeadEnd())
    {   
        String msg = "Firefighter from " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()) + " to " + grid.lastTile->getRow() + " " + grid.lastTile->getColumn();
        comms.meshPush(msg, BRIDGE_NAME); 
        int last_row = grid.currentTile->getRow();
        int last_col = grid.currentTile->getColumn();
        grid.currentTile = grid.lastTile;   
        grid.lastTile = grid.getTile(last_row, last_col);
    }
    else 
    {
        while(true)
        {
            int direction = dist(gen);
            Tile* nextTile = nullptr;
            if (grid.getNextTile(direction, nextTile))
            {
                move(nextTile);
                break;
            }
        }
    }
    changeState();
}

void Firefighter::moveToTarget()
{
    if (grid.currentTile != grid.targetTile && grid.pathToTarget.empty())
    {
        grid.bfsTo(grid.targetTile);
    }
    if (grid.currentTile == grid.targetTile)    
    {         
        comms.meshPush("Arrived", leaderID); 
        setLEDColor(255,0,0);
        state = State::WAITING;
    } else {
        move(grid.pathToTarget.front());
        grid.pathToTarget.erase(grid.pathToTarget.begin());
    }
}

void Firefighter::extinguishFire()
{
    //Serial.println("Target tile i extinguish fire:" + String(targetTile->getRow()) + " " + String(targetTile->getColumn()) + "\n");

    grid.targetTile->removeEvent(Event::FIRE);
    grid.targetTile->addEvent(Event::SMOKE);
    changeState();
    String msg = "Fire putout " + String(grid.targetTile->getRow()) + " " + String(grid.targetTile->getColumn());
    comms.meshPush(msg, 1); // Broadcast including bridge
}

void Firefighter::extinguishSmoke()
{
    //Serial.println("Target tile i extinguish smoke:" + String(targetTile->getRow()) + " " + String(targetTile->getColumn()) + "\n");
    grid.targetTile->removeEvent(Event::SMOKE);
    changeState();
    String msg = "Smoke putout " + String(grid.targetTile->getRow()) + " " + String(grid.targetTile->getColumn());
    comms.meshPush(msg, 1); // Broadcast including bridge
}

void Firefighter::moveHazmat()
{
    // Om brandmannen är vid exitTile med HAZMAT-materialet
    if (grid.currentTile->hasEvent(Event::HAZMAT) && grid.currentTile == grid.exitTile)
    {
        grid.currentTile->removeEvent(Event::HAZMAT);  // Ta bort HAZMAT från rutan.
        comms.meshPush("Hazmat saved " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()), BRIDGE_NAME); 
        changeState();  // Byt state.
    }
    // Om brandmannen har HAZMAT på sin nuvarande ruta men inte är vid exitTile
    else if (grid.currentTile->hasEvent(Event::HAZMAT))
    {
        grid.currentTile->removeEvent(Event::HAZMAT);  // Ta bort HAZMAT temporärt.
        Tile* nextStep = grid.pathToTarget.front(); 
        move(nextStep);
        String msg = "Hazmat from " + String(grid.lastTile->getRow()) + " " + String(grid.lastTile->getColumn()) + " to " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn());
        comms.meshPush(msg, BRIDGE_NAME); 
        grid.pathToTarget.erase(grid.pathToTarget.begin());
        grid.currentTile->addEvent(Event::HAZMAT);  // Lägg tillbaka HAZMAT på rutan.
    }
    //FF flyttar till rutan med hazmat
    else
    {
        move(grid.targetTile);
        grid.bfsTo(grid.exitTile);  // Beräkna kortaste vägen till exitTile.
        grid.pathToTarget.erase(grid.pathToTarget.begin());  // Ta bort det aktuella steget från vägen.
        comms.meshPush("RemoveHazmat " + String(grid.targetTile->getRow()) + " " + String(grid.targetTile->getColumn()), 0); // To broadcast
    }
}

void Firefighter::rescuePerson()
{
    if (grid.currentTile->hasEvent(Event::VICTIM) && grid.currentTile == grid.exitTile)
    {
        grid.currentTile->removeEvent(Event::VICTIM);
        comms.meshPush("Victim saved " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()), BRIDGE_NAME); 
        hasMission = false;
        teamArrived = false;
        changeState();
    }
    // Om brandmannen har ett offer men inte är vid exitTile
    else if (grid.currentTile->hasEvent(Event::VICTIM))
    {
        grid.currentTile->removeEvent(Event::VICTIM);
        Tile* nextStep = grid.pathToTarget.front();  // Hämta nästa steg.
        move(nextStep);  // Flytta till nästa ruta.
        grid.pathToTarget.erase(grid.pathToTarget.begin());
        grid.currentTile->addEvent(Event::VICTIM);
        comms.meshPush("Victim from " + String(grid.lastTile->getRow()) + " " + String(grid.lastTile->getColumn()) + " to " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()), BRIDGE_NAME);
    }
}

void Firefighter::wait()
{
    if (teamArrived) 
    {
        printToDisplay("Team has arrived");
        setLEDOff();
        state = State::RESCUING_PERSON;
    }
    else if (nbrFirefighters == 2) 
    {
        for(uint32_t member : teamMembers)
        {
            comms.meshPush("TeamArrived", member);
        }
        nbrFirefighters = 1;
        teamArrived = true;
        positionsList.clear();
    }
}

void Firefighter::TeamArrived()
{
    teamArrived = true;
    grid.bfsTo(grid.exitTile);
}

void Firefighter::startMission()
{
    comms.meshPush("Yes", leaderID);

    if (state == State::MOVING_HAZMAT) 
    {
        comms.meshPush("Hazmat " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()), 0); // To broadcast  
    }
    grid.targetTile = grid.getTile(missionTargetRow, missionTargetColumn);
    grid.getTile(missionTargetRow, missionTargetColumn)->addEvent(Event::VICTIM);
    hasMission = true;
    state = State::MOVING_TO_TARGET; 
}

void Firefighter::Die(int row, int column)
{
    if (grid.currentTile->getRow() == row && grid.currentTile->getColumn() == column)
    {
        state = State::DEAD;
    }
}
        
void Firefighter::Tick() 
{
    if (pendingHelp) 
    {
        tickCounter++;
        if (tickCounter >= 3) 
        {
            comms.meshPush("No", leaderID);
            tickCounter = 0;
            pendingHelp = false;
        }
    }
    if (state == State::SEARCHING)
    {
        if (grid.atDeadEnd())
        {
            if (grid.lastTile->hasEvent(Event::FIRE))
            {
                state = State::PUTTING_OUT_FIRE;
                grid.targetTile = grid.lastTile;
            }
        }
    }
    switch (state) 
    {
        case State::SEARCHING:
            //Serial.println("SEARCHING\n");
            searchForTarget();
            break;
        case State::MOVING_TO_TARGET:
            //Serial.println("MOVING_TO_TARGET\n");
            moveToTarget();
            break;
        case State::PUTTING_OUT_FIRE:
            //Serial.println("PUTTING_OUT_FIRE\n");
            extinguishFire();
            break;
        case State::PUTTING_OUT_SMOKE:
            //Serial.println("PUTTING_OUT_SMOKE\n");
            extinguishSmoke();
            break;
        case State::MOVING_HAZMAT:
            //Serial.println("MOVING_HAZMAT\n");
            moveHazmat();
            break;
        case State::RESCUING_PERSON:
            //Serial.println("RESCUING_PERSON\n");
            rescuePerson();
            break;  
        case State::DEAD:
            //Serial.println("DEAD\n");
            break; 
        case State::WAITING:
            //Serial.println("WAITING\n");
            wait();
            break;                 
    }
}
