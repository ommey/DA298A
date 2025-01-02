#include "Firefighter.h"

Firefighter::Firefighter() : gen(esp_random()), dist(1, 4)
{
    changeState();
}

void Firefighter::move(const Tile* destination)
{  
    grid.lastTile = grid.currentTile;
    grid.currentTile = grid.getTile(destination->getRow(), destination->getColumn());
    String msg = "Firefighter from " + String(grid.lastTile->getRow()) + " " + String(grid.lastTile->getColumn()) + " to " + grid.currentTile->getRow() + " " + grid.currentTile->getColumn();
    messagesToBridge.push(msg);
}

void Firefighter::changeState()
{
    if (hasMission) 
    {
      state = State::MOVING_TO_TARGET;
    }
    else if (grid.checkForEvent(Event::VICTIM))
    {
      String msg = "RemoveVictim " + String(grid.targetTile->getRow()) + " " + String(grid.targetTile->getColumn());
      messagesToBroadcast.push(msg);
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
        String msg = "Firefighter from " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()) + " to " + grid.lastTile->getRow() + " " + grid.lastTile->getColumn();
        messagesToBridge.push(msg);
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
    if (grid.currentTile != grid.targetTile && pathToTarget.empty())
    {
        grid.bfsTo(grid.targetTile);
    }
    if (grid.currentTile == grid.targetTile)    
    {         
        messagesToNode.push(std::make_pair(leaderID, "Arrived"));
        setLEDColor(255,0,0);
        state = State::WAITING;
    } else {
        move(pathToTarget.front());
        pathToTarget.erase(pathToTarget.begin());
    }
}

void Firefighter::extinguishFire()
{
    grid.targetTile->removeEvent(Event::FIRE);
    grid.targetTile->addEvent(Event::SMOKE);
    changeState();
    String msg = "Fire putout " + String(grid.targetTile->getRow()) + " " + String(grid.targetTile->getColumn());
    messagesToBridge.push(msg);
    messagesToBroadcast.push(msg);
}

void Firefighter::extinguishSmoke()
{
    grid.targetTile->removeEvent(Event::SMOKE);
    changeState();
    String msg = "Smoke putout " + String(grid.targetTile->getRow()) + " " + String(grid.targetTile->getColumn());
    messagesToBridge.push(msg);
    messagesToBroadcast.push(msg);
}

void Firefighter::moveHazmat()
{
    // Om brandmannen är vid exitTile med HAZMAT-materialet
    if (grid.currentTile->hasEvent(Event::HAZMAT) && grid.currentTile == grid.exitTile)
    {
        grid.currentTile->removeEvent(Event::HAZMAT);  // Ta bort HAZMAT från rutan.
        messagesToBridge.push("Hazmat saved " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()));
        changeState();  // Byt state.
    }
    // Om brandmannen har HAZMAT på sin nuvarande ruta men inte är vid exitTile
    else if (grid.currentTile->hasEvent(Event::HAZMAT))
    {
        grid.currentTile->removeEvent(Event::HAZMAT);  // Ta bort HAZMAT temporärt.
        Tile* nextStep = pathToTarget.front(); 
        move(nextStep);
        String msg = "Hazmat from " + String(grid.lastTile->getRow()) + " " + String(grid.lastTile->getColumn()) + " to " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn());
        messagesToBridge.push(msg);
        pathToTarget.erase(pathToTarget.begin());
        grid.currentTile->addEvent(Event::HAZMAT);  // Lägg tillbaka HAZMAT på rutan.
    }
    else
    {
        move(grid.targetTile);
        grid.bfsTo(grid.exitTile);  // Beräkna kortaste vägen till exitTile.
        pathToTarget.erase(pathToTarget.begin());  // Ta bort det aktuella steget från vägen.
        messagesToBroadcast.push("RemoveHazmat " + String(grid.targetTile->getRow()) + " " + String(grid.targetTile->getColumn()));

    }
}

void Firefighter::rescuePerson()
{
    if (grid.currentTile->hasEvent(Event::VICTIM) && grid.currentTile == grid.exitTile)
    {
        grid.currentTile->removeEvent(Event::VICTIM);
        messagesToBridge.push("Victim saved " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()));
        hasMission = false;
        teamArrived = false;
        changeState();
    }
    // Om brandmannen har ett offer men inte är vid exitTile
    else if (grid.currentTile->hasEvent(Event::VICTIM))
    {
        String msg = "Victim from " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()) + " to ";
        grid.currentTile->removeEvent(Event::VICTIM);
        Tile* nextStep = pathToTarget.front();  // Hämta nästa steg.
        move(nextStep);  // Flytta till nästa ruta.
        pathToTarget.erase(pathToTarget.begin());
        grid.currentTile->addEvent(Event::VICTIM);
        msg += String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn());
        messagesToBridge.push(msg);
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
            messagesToNode.push(std::make_pair(member, "TeamArrived"));
        }
        nbrFirefighters = 1;
        teamArrived = true;
    }
}

void Firefighter::TeamArrived()
{
    teamArrived = true;
    grid.bfsTo(grid.exitTile);
}

void Firefighter::startMission(int row, int column)
{
    if (state == State::MOVING_HAZMAT) {
        messagesToBroadcast.push("Hazmat " + String(grid.currentTile->getRow()) + " " + String(grid.currentTile->getColumn()));
    }
    grid.targetTile = grid.getTile(row, column);
    grid.getTile(row, column)->addEvent(Event::VICTIM);
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

void Firefighter::registerSerialOutput(QueueHandle_t *serialOutputQueue)
{
    this->serialOutputQueue = serialOutputQueue;
}

void Firefighter::registerMeshOutput(QueueHandle_t *meshOutPutQueue)
{
    this->meshOutPutQueue = meshOutPutQueue;
}

 void Firefighter::enqueueMeshOutput(const Message &msg)
{
    if (msg.message != "") 
    {
        if (xQueueSend(*meshOutputQueue, &msg, 10) != pdPASS) 
        {
            Serial.println("Failed to add to mesh queue");
        }
    }
}

void Firefighter::enqueueSerialOutput(const String &msg)
{
    if (msg != "") 
    {
        char msgChar[256];
        msg.toCharArray(msgChar, sizeof(msgChar));
        if (xQueueSend(*serialOutPutQueue, &msgChar, 10) != pdPASS) 
        {
            Serial.println("Failed to add to serial queue");
        }
    }
}
        
void Firefighter::Tick() 
{
    if (pendingHelp) {
        tickCounter++;
        if (tickCounter >= 3) {
            messagesToNode.push(std::make_pair(leaderID, "No"));
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
