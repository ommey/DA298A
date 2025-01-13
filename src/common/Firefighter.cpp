#include "Firefighter.h"

Firefighter::Firefighter() : gen(esp_random()), dist(1, 4) // TODO: testar annan seed för random og="rd"
{
    // Allokera minne för varje Tile och spara pekarna i grid
    for (int row = 0; row < 6; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            grid[row][col] = new Tile(row, col);
        }
    }

    this->id = 0;
    this->currentTile = grid[3][3];  // Pekar på första tile
    this->lastTile = grid[3][3];
    this->targetTile = grid[0][0];
    this->exitTile = grid[0][3]; 
    this->hasMission = false;
    this->nbrFirefighters = 1;

    addWalls();
    changeState();
}

void Firefighter::setId(int id)
{
    this->id = id;
}

int Firefighter::getId() const 
{
    return id;
}    

void Firefighter::move(const Tile* destination)
{  
    lastTile = currentTile;
    currentTile = grid[destination->getRow()][destination->getColumn()];
    String msg = "Firefighter from " + String(lastTile->getRow()) + " " + String(lastTile->getColumn()) + " to " + currentTile->getRow() + " " + currentTile->getColumn();
    messagesToBridge.push(msg);
}

void Firefighter::changeState()
{
    if (hasMission) 
    {
      //Serial.printf("Goes to target\n");
      //printToDisplay("Goes to target");
      state = State::MOVING_TO_TARGET;
    }
    else if (checkForEvent(currentTile, Event::VICTIM))
    {
      //Serial.printf("Goes to picking up person\n");
      //printToDisplay("Goes to picking up person");
      String msg = "RemoveVictim " + String(targetTile->getRow()) + " " + String(targetTile->getColumn());
      messagesToBroadcast.push(msg);
      state = State::MOVING_TO_TARGET; 
    }
    else if (checkForEvent(currentTile, Event::FIRE)) 
    {
      //Serial.printf("Goes to putting out fire\n");
      //printToDisplay("Goes to putting out fire");
      state = State::PUTTING_OUT_FIRE;
    } 
    else if (checkForEvent(currentTile, Event::SMOKE))
    {
      //Serial.printf("Goes to putting out smoke\n");
      //printToDisplay("Goes to putting out smoke");
      state = State::PUTTING_OUT_SMOKE;
    } 
    else if (checkForEvent(currentTile, Event::HAZMAT))
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
    //Serial.println("Target tile i search: " + String(targetTile->getRow()) + " " + String(targetTile->getColumn()));

    if (atDeadEnd())
    {   
        String msg = "Firefighter from " + String(currentTile->getRow()) + " " + String(currentTile->getColumn()) + " to " + lastTile->getRow() + " " + lastTile->getColumn();
        messagesToBridge.push(msg);
        int last_row = currentTile->getRow();
        int last_col = currentTile->getColumn();
        currentTile = lastTile;   
        lastTile = grid[last_row][last_col];
    }
    else 
    {
        while(true)
        {
            int direction = dist(gen);
            if (direction == 1 && !currentTile->hasWall(Wall::NORTH) && currentTile->getRow() > 0 
            && !grid[currentTile->getRow() - 1][currentTile->getColumn()]->hasEvent(Event::FIRE) 
            && grid[currentTile->getRow() - 1][currentTile->getColumn()] != lastTile)
            {
                move(grid[currentTile->getRow() - 1][currentTile->getColumn()]);
                break;
            }
            else if (direction == 2 && !currentTile->hasWall(Wall::EAST) && currentTile->getColumn() < 7
            && !grid[currentTile->getRow()][currentTile->getColumn() + 1]->hasEvent(Event::FIRE)
            && grid[currentTile->getRow()][currentTile->getColumn() + 1] != lastTile)
            {
                move(grid[currentTile->getRow()][currentTile->getColumn() + 1]);
                break;
            }
            else if (direction == 3 && !currentTile->hasWall(Wall::SOUTH) 
            && currentTile->getRow() < 5 && !grid[currentTile->getRow() + 1][currentTile->getColumn()]->hasEvent(Event::FIRE)
            && grid[currentTile->getRow() + 1][currentTile->getColumn()] != lastTile)
            {
                move(grid[currentTile->getRow() + 1][currentTile->getColumn()]);
                break;
            }
            else if (direction == 4 && !currentTile->hasWall(Wall::WEST) && currentTile->getColumn() > 0 
            && !grid[currentTile->getRow()][currentTile->getColumn() - 1]->hasEvent(Event::FIRE)
            && grid[currentTile->getRow()][currentTile->getColumn() - 1] != lastTile)
            {
                move(grid[currentTile->getRow()][currentTile->getColumn() - 1]);
                break;
            }
        }
    }
    changeState();
}

void Firefighter::moveToTarget()
{
    if (currentTile != targetTile && pathToTarget.empty())
    {
        bfsTo(targetTile);
    }
    if (currentTile == targetTile)    
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
    //Serial.println("Target tile i extinguish fire:" + String(targetTile->getRow()) + " " + String(targetTile->getColumn()) + "\n");

    targetTile->removeEvent(Event::FIRE);
    targetTile->addEvent(Event::SMOKE);
    changeState();
    String msg = "Fire putout " + String(targetTile->getRow()) + " " + String(targetTile->getColumn());
    messagesToBridge.push(msg);
    messagesToBroadcast.push(msg);
}

void Firefighter::extinguishSmoke()
{
    //Serial.println("Target tile i extinguish smoke:" + String(targetTile->getRow()) + " " + String(targetTile->getColumn()) + "\n");
    targetTile->removeEvent(Event::SMOKE);
    changeState();
    String msg = "Smoke putout " + String(targetTile->getRow()) + " " + String(targetTile->getColumn());
    messagesToBridge.push(msg);
    messagesToBroadcast.push(msg);
}

void Firefighter::moveHazmat()
{
    // Om brandmannen är vid exitTile med HAZMAT-materialet
    if (currentTile->hasEvent(Event::HAZMAT) && currentTile == exitTile)
    {
        currentTile->removeEvent(Event::HAZMAT);  // Ta bort HAZMAT från rutan.
        messagesToBridge.push("Hazmat saved " + String(currentTile->getRow()) + " " + String(currentTile->getColumn()));
        changeState();  // Byt state.
    }
    // Om brandmannen har HAZMAT på sin nuvarande ruta men inte är vid exitTile
    else if (currentTile->hasEvent(Event::HAZMAT))
    {
        currentTile->removeEvent(Event::HAZMAT);  // Ta bort HAZMAT temporärt.
        Tile* nextStep = pathToTarget.front(); 
        move(nextStep);
        String msg = "Hazmat from " + String(lastTile->getRow()) + " " + String(lastTile->getColumn()) + " to " + String(currentTile->getRow()) + " " + String(currentTile->getColumn());
        messagesToBridge.push(msg);
        pathToTarget.erase(pathToTarget.begin());
        currentTile->addEvent(Event::HAZMAT);  // Lägg tillbaka HAZMAT på rutan.
    }
    //FF flyttar till rutan med hazmat
    else
    {
        move(targetTile);
        bfsTo(exitTile);  // Beräkna kortaste vägen till exitTile.
        pathToTarget.erase(pathToTarget.begin());  // Ta bort det aktuella steget från vägen.
        messagesToBroadcast.push("RemoveHazmat " + String(targetTile->getRow()) + " " + String(targetTile->getColumn()));

    }
}

void Firefighter::rescuePerson()
{
    if (currentTile->hasEvent(Event::VICTIM) && currentTile == exitTile)
    {
        currentTile->removeEvent(Event::VICTIM);
        messagesToBridge.push("Victim saved " + String(currentTile->getRow()) + " " + String(currentTile->getColumn()));
        hasMission = false;
        teamArrived = false;
        changeState();
    }
    // Om brandmannen har ett offer men inte är vid exitTile
    else if (currentTile->hasEvent(Event::VICTIM))
    {
        String msg = "Victim from " + String(currentTile->getRow()) + " " + String(currentTile->getColumn()) + " to ";
        currentTile->removeEvent(Event::VICTIM);
        Tile* nextStep = pathToTarget.front();  // Hämta nästa steg.
        move(nextStep);  // Flytta till nästa ruta.
        pathToTarget.erase(pathToTarget.begin());
        currentTile->addEvent(Event::VICTIM);
        msg += String(currentTile->getRow()) + " " + String(currentTile->getColumn());
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
            // lopa igenom alla medlemmar i teamet och skicka meddelande till dem
            messagesToNode.push(std::make_pair(member, "TeamArrived"));
        }
        nbrFirefighters = 1;
        teamArrived = true;
    }
}

void Firefighter::TeamArrived()
{
    teamArrived = true;
    bfsTo(exitTile);
}

void Firefighter::startMission(int row, int column)
{
    if (state == State::MOVING_HAZMAT) {
        messagesToBroadcast.push("Hazmat " + String(currentTile->getRow()) + " " + String(currentTile->getColumn()));
    }
    targetTile = grid[row][column];
    grid[row][column]->addEvent(Event::VICTIM);
    hasMission = true;
    state = State::MOVING_TO_TARGET; 
}

void Firefighter::Die(int row, int column)
{
    if (currentTile->getRow() == row && currentTile->getColumn() == column)
    {
        state = State::DEAD;
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
        if (atDeadEnd())
        {
            if (lastTile->hasEvent(Event::FIRE))
            {
                state = State::PUTTING_OUT_FIRE;
                targetTile = lastTile;
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

Firefighter::~Firefighter() {
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 8; ++col) {
            delete grid[row][col]; // Frigör varje dynamiskt allokerad Tile
            grid[row][col] = nullptr; // Bra vana att nullställa pekare
        }
    }
}

void Firefighter::printGrid() {
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (grid[row][col]->hasEvent(Event::FIRE)) 
            {
                Serial.print("Tile has fire: " + String(row) + " " + String(col));
            } 
        }
    }
}
