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
    this->exitTile = grid[0][0]; 
    this->hasMission = false;
    this->nbrFirefighters = 1;

    changeState();
    
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

void Firefighter::bfsTo(Tile* destination)
{
    if (currentTile == destination) return;  // Om vi redan är vid målet, gör ingenting.

    std::queue<Tile*> toVisit;  // BFS-kö för att hålla reda på vilka rutor som ska utforskas.
    std::unordered_map<Tile*, Tile*> parent;  // För att återskapa vägen från destination tillbaka till start.
    std::unordered_map<Tile*, bool> visited;  // Markera vilka rutor vi har besökt.
    pathToTarget.clear();  // Rensa eventuell tidigare beräknad väg.

    // Starta BFS från nuvarande ruta
    toVisit.push(currentTile);
    visited[currentTile] = true;
    parent[currentTile] = nullptr;

    bool found = false;  // Flagga för att hålla koll på om destinationen har hittats.

    while (!toVisit.empty() && !found)
    {
        Tile* tile = toVisit.front();  // Hämta den första rutan i kön.
        toVisit.pop();  // Ta bort rutan från kön.

        // Definiera möjliga riktningar: NORTH, EAST, SOUTH, WEST.
        std::vector<std::pair<int, int>> directions = {
            {-1, 0}, {0, 1}, {1, 0}, {0, -1}  // (-1,0)=NORTH, (0,1)=EAST, (1,0)=SOUTH, (0,-1)=WEST.
        };

        for (auto& dir : directions)  // Gå igenom alla riktningar.
        {
            int newRow = tile->getRow() + dir.first;  // Beräkna ny rad.
            int newCol = tile->getColumn() + dir.second;  // Beräkna ny kolumn.

            // Kontrollera att den nya positionen är inom rutnätets gränser.
            if (newRow >= 0 && newRow < 6 && newCol >= 0 && newCol < 8)
            {
                Tile* neighbor = grid[newRow][newCol];  // Hämta grannen från rutnätet.

                // Kontrollera att grannen:
                // 1. Inte är besökt.
                // 2. Inte har en vägg i den aktuella riktningen.
                if (!visited[neighbor] &&
                    !tile->hasWall(static_cast<Wall>(
                        dir.first == -1 ? Wall::NORTH :
                        dir.first == 1 ? Wall::SOUTH :
                        dir.second == 1 ? Wall::EAST : Wall::WEST)))
                {
                    visited[neighbor] = true;  // Markera grannen som besökt.
                    parent[neighbor] = tile;  // Spara varifrån vi kom.
                    toVisit.push(neighbor);  // Lägg grannen i kön.

                    // Kontrollera om vi har nått destinationen.
                    if (neighbor == destination)
                    {
                        found = true;
                        break;
                    }
                }
            }
        }
    }

    // Om destinationen hittades, rekonstruera vägen.
    if (found)
    {
        Tile* step = destination;  // Börja från destinationen.
        while (step != nullptr)  // Backtracka tills vi når startpunkten.
        {
            pathToTarget.push_back(step);  // Lägg till rutan i vägen.
            step = parent[step];  // Gå till föräldern.
        }
        std::reverse(pathToTarget.begin(), pathToTarget.end());  // Vänd vägen så att den går från start → mål.
    }
}


void Firefighter::move(const Tile* destination)
{  
    lastTile = currentTile;
    currentTile = grid[destination->getRow()][destination->getColumn()];
    String msg = "Firefighter from " + String(lastTile->getRow()) + " " + String(lastTile->getColumn()) + " to " + currentTile->getRow() + " " + currentTile->getColumn();
    messagesToBridge.push(msg);
}


bool Firefighter::checkForEvent(Tile* tile, Event event)
{ 
    bool hasEvent = false;

    if (tile->hasEvent(event))
    {
        targetTile = tile;
        hasEvent = true;
    }    
    else if (!tile->hasWall(Wall::NORTH) && grid[tile->getRow() - 1][tile->getColumn()]->hasEvent(event))
    {
        targetTile = grid[tile->getRow() - 1][tile->getColumn()];
        hasEvent = true; 
    }
    else if (!tile->hasWall(Wall::EAST) && grid[tile->getRow()][tile->getColumn() + 1]->hasEvent(event))
    {
        targetTile = grid[tile->getRow()][tile->getColumn() + 1];
        hasEvent = true;
    }
    else if(!tile->hasWall(Wall::SOUTH) && grid[tile->getRow() + 1][tile->getColumn()]->hasEvent(event))
    {
        targetTile = grid[tile->getRow() + 1][tile->getColumn()];
        hasEvent = true;
    }
    else if (!tile->hasWall(Wall::WEST) && grid[tile->getRow()][tile->getColumn() - 1]->hasEvent(event))
    {
        targetTile = grid[tile->getRow()][tile->getColumn() - 1];
        hasEvent = true;
    }

  return hasEvent;
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
    if (walls == 3)
    {
        return true;
    } 
    return false;
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
        //Serial.printf("\nTeam has arrived");
        //printToDisplay("Team has arrived");
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
    //Serial.printf("Coordinates: %d, %d\n", currentTile->getRow(), currentTile->getColumn());
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
