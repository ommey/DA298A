#include "Firefighter.h"
#include "Tile.h"

Firefighter::Firefighter() : gen(rd()), dist(1, 4) 
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
    this->currentTile = grid[0][0];  // Pekar på första tile
    this->lastTile = grid[0][0];
    this->targetTile = grid[0][0];
    this->exitTile = grid[0][0]; 
    this->state = State::SEARCHING;
    this->hasMission = false;
    

    addWalls();
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

    int new_row = currentTile->getRow();  
    int new_column = currentTile->getColumn(); 
          
    if (destination->getColumn() < currentTile->getColumn()) 
    {
        new_column = currentTile->getColumn() - 1;
    }
    else if (destination->getColumn() > currentTile->getColumn()) 
    {
        new_column = currentTile->getColumn() + 1;
    } 

    if (currentTile->getColumn() == new_column && destination->getRow() < currentTile->getRow()) 
    {
        new_row = currentTile->getRow() - 1;
    }
    else if (currentTile->getColumn() == new_column && destination->getRow() > currentTile->getRow()) 
    {
        new_row = currentTile->getRow() + 1;
    }
    
    if (new_row < 0 || new_row >= 6 || new_column < 0 || new_column >= 8)
    {
        Serial.println("Fel: ny position utanför gränserna.");
        return;
    }

    currentTile = grid[new_row][new_column];
    String msg = "Firefighter from " + String(lastTile->getRow()) + " " + String(lastTile->getColumn()) + " to " + currentTile->getRow() + " " + currentTile->getColumn();
    messagesToBridge.push(msg);
}


bool Firefighter::ChangeState(Tile* tile)
{
    bool hasEvent = true;

    if (hasMission)
    {
        state = State::MOVING_TO_TARGET;
    }
    else if (tile->hasEvent(Event::VICTIM))
    {
        state = State::WAITING;
        targetTile = tile;
    }
    else if (tile->hasEvent(Event::FIRE))
    {
        state = State::PUTTING_OUT_FIRE;
        targetTile = tile;
    }
    else if (tile->hasEvent(Event::SMOKE))
    {
        state = State::PUTTING_OUT_SMOKE;
        targetTile = tile;
    }
    else if (tile->hasEvent(Event::HAZMAT))
    {
        state = State::MOVING_HAZMAT;
        targetTile = tile;
    }
    else { hasEvent = false; }
    return hasEvent; 
}  

bool Firefighter::CheckSurroundingsForEvent()
{
    if (ChangeState(currentTile))
    {
        Serial.println("Hittade target på samma tile");
        return true; 
    }
    else if (!currentTile->hasWall(Wall::NORTH) && currentTile->getRow() > 0)
    {
        if(ChangeState(grid[currentTile->getRow() - 1][currentTile->getColumn()]))
        {
            Serial.println("Hittade target på norr");
            return true;
        }
    }
    else if (!currentTile->hasWall(Wall::EAST) && currentTile->getColumn() < 7)
    {
        if(ChangeState(grid[currentTile->getRow()][currentTile->getColumn() + 1]))
        {
            Serial.println("Hittade target på öst");
            return true;
        }
    }
    else if (!currentTile->hasWall(Wall::SOUTH) && currentTile->getRow() < 5)
    {
        if(ChangeState(grid[currentTile->getRow() + 1][currentTile->getColumn()]))
        {
            Serial.println("Hittade target på söder");
            return true;
        }
    }
    else if (!currentTile->hasWall(Wall::WEST))
    {
        if(ChangeState(grid[currentTile->getRow()][currentTile->getColumn() - 1]) && currentTile->getColumn() > 0)
        {
            Serial.println("Hittade target på väst");
            return true;
        }
    }     
    return false;   
}

bool Firefighter::atDeadEnd()
{
    int walls = 0;

    if(currentTile->hasWall(Wall::NORTH))
    {
        walls++;
    }
    if(currentTile->hasWall(Wall::EAST))
    {
        walls++;
    }
    if(currentTile->hasWall(Wall::SOUTH))
    {
        walls++;
    }
    if(currentTile->hasWall(Wall::WEST))
    {
        walls++;
    }
    if(walls == 3)
    {
        return true;
    }
}


void Firefighter::searchForTarget()
{
    Serial.println("Target tile i search:" + String(targetTile->getRow()) + " " + String(targetTile->getColumn()));

    if (atDeadEnd())
    {
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
            if (direction == 1 && !currentTile->hasWall(Wall::NORTH) && currentTile->getRow() > 0 && !grid[currentTile->getRow() - 1][currentTile->getColumn()]->hasEvent(Event::FIRE))
            {
                move(grid[currentTile->getRow() - 1][currentTile->getColumn()]);
                break;
            }
            else if (direction == 2 && !currentTile->hasWall(Wall::EAST) && currentTile->getColumn() < 7 && !grid[currentTile->getRow()][currentTile->getColumn() + 1]->hasEvent(Event::FIRE))
            {
                move(grid[currentTile->getRow()][currentTile->getColumn() + 1]);
                break;
            }
            else if (direction == 3 && !currentTile->hasWall(Wall::SOUTH) && currentTile->getRow() < 5 && !grid[currentTile->getRow() + 1][currentTile->getColumn()]->hasEvent(Event::FIRE))
            {
                move(grid[currentTile->getRow() + 1][currentTile->getColumn()]);
                break;
            }
            else if (direction == 4 && !currentTile->hasWall(Wall::WEST) && currentTile->getColumn() > 0 && !grid[currentTile->getRow()][currentTile->getColumn() - 1]->hasEvent(Event::FIRE))
            {
                move(grid[currentTile->getRow()][currentTile->getColumn() - 1]);
                break;
            }
        }
    }
    CheckSurroundingsForEvent();
}

void Firefighter::moveToTarget()
{
    if (currentTile != targetTile)
    {
        move(targetTile);
    } 
    else    
    { 
        for(const auto& [key, value] : team)
        {
            messagesToNode.push("Arrived");
        }
        if (!teamArrived())
        {
            state = State::WAITING;
        }
        else 
        {
            state = State::RESCUING_PERSON;
        }     
    }
}

void Firefighter::extinguishFire()
{
    Serial.println("Target tile i extinguish fire:" + String(targetTile->getRow()) + " " + String(targetTile->getColumn()));

    targetTile->removeEvent(Event::FIRE);
    targetTile->addEvent(Event::SMOKE);
    if (!CheckSurroundingsForEvent())
    {
        state = State::SEARCHING;
    }
    String msg = "Fire putout " + String(targetTile->getRow()) + " " + String(targetTile->getColumn());
    messagesToBridge.push(msg);
}

void Firefighter::extinguishSmoke()
{
    Serial.println("Target tile i extinguish smoke:" + String(targetTile->getRow()) + " " + String(targetTile->getColumn()));
    targetTile->removeEvent(Event::SMOKE);
    if (!CheckSurroundingsForEvent())
    {
        state = State::SEARCHING;
    }
    String msg = "Smoke putout " + String(targetTile->getRow()) + " " + String(targetTile->getColumn());
    messagesToBridge.push(msg);
}

void Firefighter::moveHazmat()
{
    if (targetTile != currentTile) 
    {
        move(targetTile);
    }
    else if (currentTile->hasEvent(Event::HAZMAT) && currentTile == exitTile)
    {
        currentTile->removeEvent(Event::HAZMAT);
        messagesToBridge.push("Hazmat saved " + String(currentTile->getRow()) + " " + String(currentTile->getColumn()));
        if (!CheckSurroundingsForEvent())
        {
            state = State::SEARCHING;
        }
    }
    else if (currentTile->hasEvent(Event::HAZMAT))
    {
        currentTile->removeEvent(Event::HAZMAT);
        move(exitTile);
        currentTile->addEvent(Event::HAZMAT);
        String msg = "Hazmat from " + String(lastTile->getRow()) + " " + String(lastTile->getColumn()) + " to " + String(currentTile->getRow()) + " " + String(currentTile->getColumn());
        messagesToBridge.push(msg);
    }
}

void Firefighter::rescuePerson()
{
    if (currentTile->hasEvent(Event::VICTIM) && currentTile->getRow() == exitTile->getRow() && currentTile->getColumn() == exitTile->getColumn())
    {
        currentTile->removeEvent(Event::VICTIM);
        messagesToBridge.push("Victim saved " + String(currentTile->getRow()) + " " + String(currentTile->getColumn()));
        state = State::SEARCHING;
    }
    else if (currentTile->hasEvent(Event::VICTIM))
    {
        String msg = "Victim from " + String(currentTile->getRow()) + " " + String(currentTile->getColumn()) + " to ";
        currentTile->removeEvent(Event::VICTIM);
        move(exitTile);
        currentTile->addEvent(Event::VICTIM);
        msg += String(currentTile->getRow()) + " " + String(currentTile->getColumn());
        messagesToBridge.push(msg);
    }
}

void Firefighter::wait()
{
    if (currentTile != targetTile)
    {
        move(targetTile);
    }

    if (teamArrived())
    {
        state = State::RESCUING_PERSON;
    }
}

bool Firefighter::teamArrived()
{
    bool teamArrived = true; 

    for(const auto& [key, value] : team)
    {
        if (value == false)
        {
            teamArrived = false;
            break;
        }
    }
    return teamArrived;
}

void Firefighter::startMission(int row, int column)
{
    targetTile = grid[row][column];
    hasMission = true;
}

void Firefighter::Die()
{
    state = State::DEAD;
}
        
void Firefighter::Tick() 
{
    switch (state) 
    {
        case State::SEARCHING:
            Serial.println("SEARCHING");
            searchForTarget();
            break;
        case State::MOVING_TO_TARGET:
            Serial.println("MOVINGTOTARGET");
            moveToTarget();
            break;
        case State::PUTTING_OUT_FIRE:
            Serial.println("PUTTING_OUT_FIRE");
            extinguishFire();
            break;
        case State::PUTTING_OUT_SMOKE:
            Serial.println("PUTTING_OUT_SMOKE");
            extinguishSmoke();
            break;
        case State::MOVING_HAZMAT:
            Serial.println("MOVING_HAZMAT");
            moveHazmat();
            break;
        case State::RESCUING_PERSON:
            Serial.println("RESCUING_PERSON");
            rescuePerson();
            break;  
        case State::DEAD:
            Serial.println("DEAD");
            Die();
            break; 
        case State::WAITING:
            Serial.println("WAITING");
            wait();
            break;                 
    }
}

void Firefighter::addWalls()
{
    grid[0][0]->addWall(Wall::NORTH);
    grid[0][0]->addWall(Wall::WEST);
    grid[1][0]->addWall(Wall::WEST);
    grid[2][0]->addWall(Wall::WEST);
    grid[2][0]->addWall(Wall::SOUTH);
    grid[3][0]->addWall(Wall::WEST);
    grid[3][0]->addWall(Wall::NORTH);
    grid[4][0]->addWall(Wall::WEST);
    grid[5][0]->addWall(Wall::WEST);
    grid[5][0]->addWall(Wall::SOUTH);
    grid[5][0]->addWall(Wall::EAST);
    grid[0][1]->addWall(Wall::NORTH);
    grid[2][1]->addWall(Wall::SOUTH);
    grid[3][1]->addWall(Wall::NORTH);
    grid[4][1]->addWall(Wall::SOUTH);
    grid[4][1]->addWall(Wall::EAST);
    grid[5][1]->addWall(Wall::SOUTH);
    grid[5][1]->addWall(Wall::WEST);
    grid[5][1]->addWall(Wall::NORTH);
    grid[0][2]->addWall(Wall::NORTH);
    grid[0][2]->addWall(Wall::EAST);
    grid[2][2]->addWall(Wall::EAST);
    grid[4][2]->addWall(Wall::EAST);
    grid[4][2]->addWall(Wall::WEST);
    grid[5][2]->addWall(Wall::SOUTH);
    grid[0][3]->addWall(Wall::WEST);
    grid[0][3]->addWall(Wall::EAST);
    grid[0][3]->addWall(Wall::NORTH);
    grid[2][3]->addWall(Wall::WEST);
    grid[2][3]->addWall(Wall::EAST);
    grid[4][3]->addWall(Wall::SOUTH);
    grid[4][3]->addWall(Wall::WEST);
    grid[4][3]->addWall(Wall::EAST);
    grid[5][3]->addWall(Wall::SOUTH);
    grid[5][3]->addWall(Wall::EAST);
    grid[5][3]->addWall(Wall::NORTH);
    grid[0][4]->addWall(Wall::NORTH);
    grid[0][4]->addWall(Wall::WEST);
    grid[2][4]->addWall(Wall::WEST);
    grid[4][4]->addWall(Wall::WEST);
    grid[4][4]->addWall(Wall::SOUTH);
    grid[5][4]->addWall(Wall::EAST);
    grid[5][4]->addWall(Wall::SOUTH);
    grid[5][4]->addWall(Wall::WEST);
    grid[0][5]->addWall(Wall::NORTH);
    grid[5][5]->addWall(Wall::WEST);
    grid[5][5]->addWall(Wall::EAST);
    grid[5][5]->addWall(Wall::SOUTH);
    grid[0][6]->addWall(Wall::NORTH);
    grid[1][6]->addWall(Wall::EAST);
    grid[2][6]->addWall(Wall::EAST);
    grid[5][6]->addWall(Wall::EAST);
    grid[5][6]->addWall(Wall::SOUTH);
    grid[5][6]->addWall(Wall::WEST);
    grid[0][7]->addWall(Wall::NORTH);
    grid[0][7]->addWall(Wall::EAST);
    grid[0][7]->addWall(Wall::WEST);
    grid[1][7]->addWall(Wall::EAST);
    grid[1][7]->addWall(Wall::SOUTH);
    grid[2][7]->addWall(Wall::NORTH);
    grid[2][7]->addWall(Wall::EAST);
    grid[2][7]->addWall(Wall::WEST);
    grid[3][7]->addWall(Wall::SOUTH);
    grid[3][7]->addWall(Wall::EAST);
    grid[4][7]->addWall(Wall::NORTH);
    grid[4][7]->addWall(Wall::EAST);
    grid[5][7]->addWall(Wall::EAST);
    grid[5][7]->addWall(Wall::SOUTH);
    grid[5][7]->addWall(Wall::WEST);
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


