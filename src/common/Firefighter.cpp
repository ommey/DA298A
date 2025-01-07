#include "Firefighter.h"

Firefighter::Firefighter() : gen(esp_random()), dist(1, 4)
{
    changeState();
}

void Firefighter::move(const Tile* destination)
{  
    grid.lastTile = grid.currentTile;
    grid.currentTile = grid.getTile(destination->getRow(), destination->getColumn());
    string messageContent = "Firefighter from " + to_string(grid.lastTile->getRow()) + " " + to_string(grid.lastTile->getColumn()) + " to " + to_string(grid.currentTile->getRow()) + " " + to_string(grid.currentTile->getColumn()) ;
    enqueueMeshOutput(Message(bridgeName, messageContent.c_str())); 
}

void Firefighter::changeState()
{
    if (hasMission) 
    {
        state = State::MOVING_TO_TARGET;
    }
    else if (grid.checkForEvent(Event::VICTIM))
    {
        string messageContent = "RemoveVictim " + to_string(grid.targetTile->getRow()) + " " + to_string(grid.targetTile->getColumn());
        enqueueMeshOutput(Message(0, messageContent.c_str()));
        leaderID = 0;      
        printToDisplay("Rescuing person"); 
        state = State::MOVING_TO_TARGET; 
    }
    else if (grid.checkForEvent(Event::FIRE)) 
    {
        state = State::PUTTING_OUT_FIRE;
    } 
    else if (grid.checkForEvent(Event::SMOKE))
    {
        state = State::PUTTING_OUT_SMOKE;
    } 
    else if (grid.checkForEvent(Event::HAZMAT))
    {
        printToDisplay("Moving Hazmat");
        state = State::MOVING_HAZMAT;
    } 
    else 
    {
        state = State::SEARCHING;
    }
}

void Firefighter::searchForTarget()
{
    if (grid.atDeadEnd())
    {      
        string messageContent = "Firefighter from " + to_string(grid.currentTile->getRow()) + " " + to_string(grid.currentTile->getColumn()) + " to " + to_string(grid.lastTile->getRow()) + " " + to_string(grid.lastTile->getColumn()) ;
        enqueueMeshOutput(Message(bridgeName, messageContent.c_str()));    
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
        printToDisplay("Calculating path");
        enqueueMeshOutput(Message(bridgeName, "Calculating path"));
        grid.bfsTo(grid.targetTile);
    }
    if (grid.currentTile == grid.targetTile)    
    { 
        printToDisplay("Arrived at target"); 
        if (leaderID != 0)
        {
            enqueueMeshOutput(Message(leaderID, "Arrived")); 
        }      
        setLEDColor(255,0,0);
        state = State::WAITING;
    } else {
        printToDisplay("Else");
        move(grid.pathToTarget.front());
        grid.pathToTarget.erase(grid.pathToTarget.begin());
    }
}

void Firefighter::extinguishFire()
{
    grid.targetTile->removeEvent(Event::FIRE);
    grid.targetTile->addEvent(Event::SMOKE);
    string messageContent = "RemoveFire " + to_string(grid.targetTile->getRow()) + " " + to_string(grid.targetTile->getColumn());
    enqueueMeshOutput(Message(0, messageContent.c_str(), true));
    changeState(); 
}

void Firefighter::extinguishSmoke()
{
    grid.targetTile->removeEvent(Event::SMOKE);
    string messageContent = "RemoveSmoke " + to_string(grid.targetTile->getRow()) + " " + to_string(grid.targetTile->getColumn()) ;
    enqueueMeshOutput(Message(0, messageContent.c_str(), true)); 
    changeState();
}

void Firefighter::moveHazmat()
{
    // Om brandmannen är vid exitTile med HAZMAT-materialet
    if (grid.currentTile->hasEvent(Event::HAZMAT) && grid.currentTile == grid.exitTile)
    {
        grid.currentTile->removeEvent(Event::HAZMAT);  // Ta bort HAZMAT från rutan.
        string messageContent = "RemoveHazmat " + to_string(grid.currentTile->getRow()) + " " + to_string(grid.currentTile->getColumn());
        enqueueMeshOutput(Message(0, messageContent.c_str(), true));
        changeState();  // Byt state.
    }
    // Om brandmannen har HAZMAT på sin nuvarande ruta men inte är vid exitTile
    else if (grid.currentTile->hasEvent(Event::HAZMAT))
    {
        grid.currentTile->removeEvent(Event::HAZMAT);  // Ta bort HAZMAT temporärt.
        Tile* nextStep = grid.pathToTarget.front(); 
        move(nextStep);
        
        string messageContent = "Hazmat from " + to_string(grid.lastTile->getRow()) + " " + to_string(grid.lastTile->getColumn()) + " to " + to_string(grid.currentTile->getRow()) + " " + to_string(grid.currentTile->getColumn());       
        enqueueMeshOutput(Message(bridgeName, messageContent.c_str()));
        grid.pathToTarget.erase(grid.pathToTarget.begin());
        grid.currentTile->addEvent(Event::HAZMAT);  // Lägg tillbaka HAZMAT på rutan.
    }
    else
    {        
        move(grid.targetTile);
        grid.bfsTo(grid.exitTile);  // Beräkna kortaste vägen till exitTile.
        grid.pathToTarget.erase(grid.pathToTarget.begin());  // Ta bort det aktuella steget från vägen.
        string messageContent = "RemoveHazmat " + to_string(grid.targetTile->getRow()) + " " + to_string(grid.targetTile->getColumn()) ;
        enqueueMeshOutput(Message(0, messageContent.c_str()));
    }
}

void Firefighter::rescuePerson()
{
    if (grid.currentTile->hasEvent(Event::VICTIM) && grid.currentTile == grid.exitTile)
    {
        grid.currentTile->removeEvent(Event::VICTIM);
        string messageContent = "RemoveVictim " + to_string(grid.currentTile->getRow()) + " " + to_string(grid.currentTile->getColumn()) ;
        enqueueMeshOutput(Message(0, messageContent.c_str(), true));
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
        ostringstream oss;
        oss << "Victim from " << grid.lastTile->getRow() << ' ' << grid.lastTile->getColumn() << " to " << grid.lastTile->getRow() << ' ' << grid.lastTile->getColumn();
        string messageContent = oss.str();
        enqueueMeshOutput(Message(bridgeName, messageContent.c_str()));
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
            enqueueMeshOutput(Message(member, "TeamArrived")); 
        }
        nbrFirefighters = 1;
        teamArrived = true;
    }
    else
    {
        printToDisplay("Waiting for team");    
    }
}

void Firefighter::TeamArrived()
{
    teamArrived = true;
    grid.bfsTo(grid.exitTile);
}

void Firefighter::startMission()
{
    if (state == State::MOVING_HAZMAT) 
    {
        enqueueMeshOutput(Message(0, "Hazmat " + grid.currentTile->getRow() + ' ' + grid.currentTile->getColumn())); 
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

Firefighter::~Firefighter()
{
    delete &grid;
}

/*void Firefighter::registerSerialOutput(QueueHandle_t *serialOutputQueue)
{
    this->serialOutputQueue = serialOutputQueue;
}*/

void Firefighter::registerMeshOutput(QueueHandle_t *meshOutputQueue)
{
    this->meshOutPutQueue = meshOutputQueue;
}

void Firefighter::enqueueMeshOutput(const Message &msg)
{
    if (msg.message != "" && meshOutPutQueue != nullptr) 
    {
        if (xQueueSend(*meshOutPutQueue, &msg, 10) != pdPASS) 
        {
            Serial.println("Failed to add to mesh queue");
        }
    }
}

/*void Firefighter::enqueueSerialOutput(const String &msg)
{
    if (msg != "" && serialOutputQueue != nullptr) 
    {
        char msgChar[256];
        msg.toCharArray(msgChar, sizeof(msgChar));
        if (xQueueSend(*serialOutputQueue, &msgChar, 10) != pdPASS) 
        {
            Serial.println("Failed to add to serial queue");
        }
    }
}*/

void Firefighter::handleMessage(uint32_t from, String msg)
{
    int row = 0;
    int column = 0;
    vector<String> tokens = tokenize(msg);

      if (tokens[0] == "Tick") { Tick(); }

      else if (tokens.size() == 3 && tryParseInt(tokens[1], row) && tryParseInt(tokens[2], column)) 
      {      
        if (tokens[0] == "Fire")
        {
            grid.getTile(row, column)->addEvent(Event::FIRE);
        }
        else if (tokens[0] == "Smoke")
        {
            grid.getTile(row, column)->addEvent(Event::SMOKE);
        }
        else if (tokens[0] == "Victim")
        {
            grid.getTile(row, column)->addEvent(Event::VICTIM);
        }
        else if (tokens[0] == "Hazmat")
        {
            grid.getTile(row, column)->addEvent(Event::HAZMAT);
        } 
        else if (tokens[0] == "RemoveVictim")
        {
            grid.getTile(row, column)->removeEvent(Event::VICTIM);
        }
        else if (tokens[0] == "RemoveFire")
        {
            grid.getTile(row, column)->removeEvent(Event::FIRE);
        }
        else if (tokens[0] == "RemoveSmoke")
        {
            grid.getTile(row, column)->removeEvent(Event::SMOKE);
        }
        else if (tokens[0] == "RemoveHazmat")
        {
            grid.getTile(row, column)->removeEvent(Event::HAZMAT);
        } 
        else if (tokens[0] == "MaybeDie")
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
        else if (tokens[0] == "ReqPos") 
        {
            enqueueMeshOutput(Message(from, "Pos " + grid.currentTile->getRow() + ' ' + grid.currentTile->getColumn() ));
        }
        else if (tokens[0] == "Yes") 
        { 
            printToDisplay("Yes recieved");       
            teamMembers.push_back(from);
        }
        else if (tokens[0] == "No") 
        { 
            printToDisplay("No recieved");
            for (int i = 0; i < teamMembers.size(); i++) {
                if (positionsList[positionListCounter].first == teamMembers[i]) {
                i = 0;
                positionListCounter = (positionListCounter + 1) % positionsList.size();
                }
            }
            enqueueMeshOutput(Message(positionsList[positionListCounter].first, "Help " + grid.targetTile->getRow() + ' ' + grid.targetTile->getColumn() ));
            positionListCounter = (positionListCounter + 1) % positionsList.size();
        }
        else if (tokens[0] == "Arrived")
        {
            nbrFirefighters++;
        }
        else if (tokens[0] == "TeamArrived")
        {
            TeamArrived();
        } 
    }
}

vector<String> Firefighter::tokenize(const String& expression) 
{
    vector<String> tokens;
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

        if (value >= INT_MIN && value <= INT_MAX) {
            return true;
        }
    }
    return false; // Parsning misslyckades
}

void Firefighter::handlePositions(uint32_t from, int row, int column)
{  
  float dis = sqrt(pow(row-grid.targetTile->getRow(),2)+pow(column-grid.targetTile->getColumn(),2));
  positionsList.push_back({from, dis}); // Spara nodens position i positionsList
  if (positionsList.size() == nbrExpectedAnswers) //Check if all nodes anwsered, if true, start sorting
  { 
    sort(positionsList.begin(), positionsList.end(),
    [](const pair<uint32_t, float>& a, const pair<uint32_t, float>& b) 
    {
      return a.second < b.second; // Compare by distance
    });
    
    positionListCounter = 0;
    
    for (positionListCounter; positionListCounter < 1; positionListCounter++) 
    {
      printToDisplay("Called firefighter: " + String(positionsList[positionListCounter].first) + " with distance: " + String(positionsList[positionListCounter].second));
      enqueueMeshOutput(Message(positionsList[positionListCounter].first, "Help " + grid.targetTile->getRow() + ' ' + grid.targetTile->getColumn() ));
    }
  }
}

void Firefighter::handleHelpRequest(uint32_t from, int row, int column)
{
  leaderID = from;  
  setLEDColor(0, 0, 255);  // Blå hjälpfärg
  printToDisplay("Help request recieved");
  missionTargetRow = row;
  missionTargetColumn = column;
  tickCounter = 0;
  pendingHelp = true;
}
        
void Firefighter::Tick() 
{
    if (pendingHelp) {
        tickCounter++;
        if (tickCounter >= 3) 
        {
            enqueueMeshOutput(Message(leaderID, "No"));
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
            //Serial.println("SEARCHING");
            searchForTarget();
            break;
        case State::MOVING_TO_TARGET:
            //Serial.println("MOVING_TO_TARGET");
            moveToTarget();
            break;
        case State::PUTTING_OUT_FIRE:
            //Serial.println("PUTTING_OUT_FIRE");
            extinguishFire();
            break;
        case State::PUTTING_OUT_SMOKE:
            //Serial.println("PUTTING_OUT_SMOKE");
            extinguishSmoke();
            break;
        case State::MOVING_HAZMAT:
            //Serial.println("MOVING_HAZMAT");
            moveHazmat();
            break;
        case State::RESCUING_PERSON:
            //Serial.println("RESCUING_PERSON");
            rescuePerson();
            break;  
        case State::DEAD:
            //Serial.println("DEAD");
            break; 
        case State::WAITING:
            //Serial.println("WAITING");
            wait();
            break;                 
    }
}
