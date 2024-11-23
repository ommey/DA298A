#include "Firefighter.h"
#include "Tile.h"

Firefighter::Firefighter()
{
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 8; ++col) {
            grid[row][col] = Tile(row, col);
        }
    }

    this->id = 0;
    this->currentTile = grid[0][0];
    this->lastTile = grid[0][0];
    this->targetTile = grid[0][0];
    this->exitTile = grid[0][0]; 
    this->state = State::SEARCHING;

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

void Firefighter::move(Tile destination)
{
    String msg = "Firefighter from " + String(currentTile.getRow()) + " " + String(currentTile.getColumn()) + " to ";

    lastTile.firefighters--;
    lastTile = currentTile;

    int new_row;
    int new_column;

    if (destination.getRow() == currentTile.getRow() && destination.getColumn() == currentTile.getColumn())
    {
        state = State::SEARCHING;
    }            
    else if (destination.getRow() < currentTile.getRow()) 
    {
        new_row = currentTile.getRow() - 1;
    }
    else if (destination.getRow() > currentTile.getRow()) 
    {
        new_row = currentTile.getRow() + 1;
    }
    else if (destination.getColumn() < currentTile.getColumn()) 
    {
        new_column = currentTile.getColumn() - 1;
    }
    else if (destination.getColumn() > currentTile.getColumn()) 
    {
        new_column = currentTile.getColumn() + 1;
    }

    currentTile = grid[new_row][new_column];
    currentTile.firefighters++;

    msg += String(currentTile.getRow()) + " " + String(currentTile.getColumn());
    messagesToBridge.push(msg);
} 

bool Firefighter::ChangeState(Tile tile)
{
    targetTile = tile;

    if (tile.hasEvent(Event::VICTIM))
    {
        state = State::RESCUING_PERSON;
        return true;
    }
    else if (tile.hasEvent(Event::HAZMAT))
    {
        state = State::MOVING_HAZMAT;
        return true;
    }
    else if (tile.hasEvent(Event::FIRE))
    {
        state = State::PUTTING_OUT_FIRE;
        return true;
    }
    else if (tile.hasEvent(Event::SMOKE))
    {
        state = State::PUTTING_OUT_SMOKE;
        return true;
    }
    return false; 
}  

void Firefighter::searchForTarget()
{
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> row_dist(0, 5); 
    std::uniform_int_distribution<> col_dist(0, 7); 

    move(grid[row_dist(gen)][col_dist(gen)]);

    if (!ChangeState(currentTile))
    {
        if (!currentTile.hasWall(Wall::NORTH))
        {
            if(ChangeState(grid[currentTile.getRow() - 1][currentTile.getColumn()]))
            {
                return;
            }
        }
        else if (!currentTile.hasWall(Wall::EAST))
        {
            if(ChangeState(grid[currentTile.getRow()][currentTile.getColumn() + 1]))
            {
                return;
            }
        }
        else if (!currentTile.hasWall(Wall::SOUTH))
        {
            if(ChangeState(grid[currentTile.getRow() + 1][currentTile.getColumn()]))
            {
                return;
            }
        }
        else if (!currentTile.hasWall(Wall::WEST))
        {
            if(ChangeState(grid[currentTile.getRow()][currentTile.getColumn() - 1]))
            {
                return;
            }
        }
    }
}

void Firefighter::moveToTarget()
{
    move(targetTile);
    if (currentTile.getRow() == targetTile.getRow() && currentTile.getColumn() == targetTile.getColumn())
    {
        state = State::SEARCHING;
    }
}

void Firefighter::extinguishFire()
{
    targetTile.removeEvent(Event::FIRE);
    state = State::PUTTING_OUT_SMOKE;
    String msg = "Fire putout " + String(targetTile.getRow()) + " " + String(targetTile.getColumn());
    messagesToBridge.push(msg);
}

void Firefighter::extinguishSmoke()
{
    targetTile.removeEvent(Event::SMOKE);
    state = State::SEARCHING;
    String msg = "Smoke putout " + String(targetTile.getRow()) + " " + String(targetTile.getColumn());
    messagesToBridge.push(msg);
}

void Firefighter::moveHazmat()
{
    if (currentTile.hasEvent(Event::HAZMAT) && currentTile.getRow() == exitTile.getRow() && currentTile.getColumn() == exitTile.getColumn())
    {
        currentTile.removeEvent(Event::HAZMAT);
        messagesToBridge.push("Hazmat saved " + String(currentTile.getRow()) + " " + String(currentTile.getColumn()));
        state = State::SEARCHING;
    }
    else if (currentTile.hasEvent(Event::HAZMAT))
    {
        String msg = "Hazmat from " + String(currentTile.getRow()) + " " + String(currentTile.getColumn()) + " to ";
        currentTile.removeEvent(Event::HAZMAT);
        move(exitTile);
        currentTile.addEvent(Event::HAZMAT);
        msg += String(currentTile.getRow()) + " " + String(currentTile.getColumn());
        messagesToBridge.push(msg);
    }
}

void Firefighter::rescuePerson()
{
    if (currentTile.firefighters >= 4 && currentTile.hasEvent(Event::VICTIM) && currentTile.getRow() == exitTile.getRow() && currentTile.getColumn() == exitTile.getColumn())
    {
        currentTile.removeEvent(Event::VICTIM);
        messagesToBridge.push("Victim saved " + String(currentTile.getRow()) + " " + String(currentTile.getColumn()));
        state = State::SEARCHING;
    }
    else if (currentTile.firefighters >= 4 && currentTile.hasEvent(Event::VICTIM))
    {
        String msg = "Victim from " + String(currentTile.getRow()) + " " + String(currentTile.getColumn()) + " to ";
        currentTile.removeEvent(Event::VICTIM);
        move(exitTile);
        currentTile.addEvent(Event::VICTIM);
        msg += String(currentTile.getRow()) + " " + String(currentTile.getColumn());
        messagesToBridge.push(msg);
    }
}

void Firefighter::Die()
{
    state = State::DEAD;
}
        
void Firefighter::Tick() 
{
    switch (state) {
        case State::SEARCHING:
            searchForTarget();
            break;
        case State::MOVING:
            moveToTarget();
            break;
        case State::PUTTING_OUT_FIRE:
            extinguishFire();
            break;
        case State::PUTTING_OUT_SMOKE:
            extinguishSmoke();
            break;
        case State::MOVING_HAZMAT:
            moveHazmat();
            break;
        case State::RESCUING_PERSON:
            rescuePerson();
            break;  
        case State::DEAD:
            Die();
            break;                  
    }
}

void Firefighter::addWalls()
{
    grid[0][0].addWall(Wall::NORTH);
    grid[0][0].addWall(Wall::WEST);
    grid[1][0].addWall(Wall::WEST);
    grid[2][0].addWall(Wall::WEST);
    grid[2][0].addWall(Wall::SOUTH);
    grid[3][0].addWall(Wall::WEST);
    grid[3][0].addWall(Wall::NORTH);
    grid[4][0].addWall(Wall::WEST);
    grid[5][0].addWall(Wall::WEST);
    grid[5][0].addWall(Wall::SOUTH);
    grid[5][0].addWall(Wall::EAST);
    grid[0][1].addWall(Wall::NORTH);
    grid[2][1].addWall(Wall::SOUTH);
    grid[3][1].addWall(Wall::NORTH);
    grid[3][1].addWall(Wall::EAST);
    grid[4][1].addWall(Wall::SOUTH);
    grid[4][1].addWall(Wall::EAST);
    grid[5][1].addWall(Wall::SOUTH);
    grid[5][1].addWall(Wall::WEST);
    grid[5][1].addWall(Wall::NORTH);
    grid[0][2].addWall(Wall::NORTH);
    grid[0][2].addWall(Wall::EAST);
    grid[1][2].addWall(Wall::EAST);
    grid[2][2].addWall(Wall::EAST);
    grid[2][2].addWall(Wall::SOUTH);
    grid[3][2].addWall(Wall::EAST);
    grid[3][2].addWall(Wall::NORTH);
    grid[3][2].addWall(Wall::WEST);
    grid[4][2].addWall(Wall::EAST);
    grid[4][2].addWall(Wall::WEST);
    grid[5][2].addWall(Wall::SOUTH);
    grid[5][2].addWall(Wall::EAST);
    grid[0][3].addWall(Wall::WEST);
    grid[0][3].addWall(Wall::EAST);
    grid[0][3].addWall(Wall::NORTH);
    grid[1][3].addWall(Wall::WEST);
    grid[1][3].addWall(Wall::EAST);
    grid[2][3].addWall(Wall::WEST);
    grid[2][3].addWall(Wall::EAST);
    grid[3][3].addWall(Wall::WEST);
    grid[3][3].addWall(Wall::EAST);
    grid[4][3].addWall(Wall::SOUTH);
    grid[4][3].addWall(Wall::WEST);
    grid[4][3].addWall(Wall::EAST);
    grid[5][3].addWall(Wall::SOUTH);
    grid[5][3].addWall(Wall::EAST);
    grid[5][3].addWall(Wall::NORTH);
    grid[5][3].addWall(Wall::WEST);
    grid[0][4].addWall(Wall::NORTH);
    grid[0][4].addWall(Wall::WEST);
    grid[1][4].addWall(Wall::WEST);
    grid[2][4].addWall(Wall::WEST);
    grid[3][4].addWall(Wall::WEST);
    grid[4][4].addWall(Wall::WEST);
    grid[4][4].addWall(Wall::SOUTH);
    grid[5][4].addWall(Wall::EAST);
    grid[5][4].addWall(Wall::SOUTH);
    grid[5][4].addWall(Wall::WEST);
    grid[5][4].addWall(Wall::NORTH);
    grid[0][5].addWall(Wall::NORTH);
    grid[5][5].addWall(Wall::WEST);
    grid[5][5].addWall(Wall::EAST);
    grid[5][5].addWall(Wall::SOUTH);
    grid[0][6].addWall(Wall::NORTH);
    grid[0][6].addWall(Wall::EAST);
    grid[1][6].addWall(Wall::EAST);
    grid[2][6].addWall(Wall::EAST);
    grid[3][6].addWall(Wall::EAST);
    grid[4][6].addWall(Wall::EAST);
    grid[5][6].addWall(Wall::EAST);
    grid[5][6].addWall(Wall::SOUTH);
    grid[5][6].addWall(Wall::WEST);
    grid[0][7].addWall(Wall::NORTH);
    grid[0][7].addWall(Wall::EAST);
    grid[0][7].addWall(Wall::WEST);
    grid[1][7].addWall(Wall::EAST);
    grid[1][7].addWall(Wall::SOUTH);
    grid[1][7].addWall(Wall::WEST);
    grid[2][7].addWall(Wall::NORTH);
    grid[2][7].addWall(Wall::EAST);
    grid[2][7].addWall(Wall::WEST);
    grid[3][7].addWall(Wall::SOUTH);
    grid[3][7].addWall(Wall::EAST);
    grid[3][7].addWall(Wall::WEST);
    grid[4][7].addWall(Wall::NORTH);
    grid[4][7].addWall(Wall::EAST);
    grid[4][7].addWall(Wall::WEST);
    grid[5][7].addWall(Wall::EAST);
    grid[5][7].addWall(Wall::SOUTH);
    grid[5][7].addWall(Wall::WEST);
}