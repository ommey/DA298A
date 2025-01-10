#include "Grid.h"

Grid::Grid()
{
    // Allokera minne för varje Tile och spara pekarna i grid
    for (int row = 0; row < 6; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            grid[row][col] = new Tile(row, col);
        }
    }
    this->currentTile = grid[3][3];  
    this->lastTile = grid[3][3];
    this->targetTile = grid[0][0];
    this->exitTile = grid[0][3];
    
    addWalls();
}

void Grid::update(String event, int row, int column)
{
    if (row < 0 || row >= 6 || column < 0 || column >= 8) 
    {
    return; // Ignorera ogiltiga positioner
    }

    if (event == "Fire")
    {
        grid[row][column]->addEvent(Event::FIRE);
    }
    else if (event == "Smoke")
    {
        grid[row][column]->addEvent(Event::SMOKE);
    }
    else if (event == "Victim")
    {
        grid[row][column]->addEvent(Event::VICTIM);
    }
    else if (event == "Hazmat")
    {
        grid[row][column]->addEvent(Event::HAZMAT);
    } 
    else if (event == "RemoveVictim")
    {
        grid[row][column]->removeEvent(Event::VICTIM);
    }
    else if (event == "RemoveHazmat")
    {
        grid[row][column]->removeEvent(Event::HAZMAT);
    } 
}

Grid::~Grid() 
{    
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 8; ++col) {
            delete grid[row][col]; // Frigör varje dynamiskt allokerad Tile
            grid[row][col] = nullptr; // Bra vana att nullställa pekare
        }
    }
}

bool Grid::atDeadEnd()
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

bool Grid::getNextTile(int direction, Tile*& nextTile)
{
    if (direction == 1 && !currentTile->hasWall(Wall::NORTH) && currentTile->getRow() > 0 
    && !grid[currentTile->getRow() - 1][currentTile->getColumn()]->hasEvent(Event::FIRE) 
    && grid[currentTile->getRow() - 1][currentTile->getColumn()] != lastTile)
    {
        nextTile = grid[currentTile->getRow() - 1][currentTile->getColumn()];
        return true;
    }
    else if (direction == 2 && !currentTile->hasWall(Wall::EAST) && currentTile->getColumn() < 7
    && !grid[currentTile->getRow()][currentTile->getColumn() + 1]->hasEvent(Event::FIRE)
    && grid[currentTile->getRow()][currentTile->getColumn() + 1] != lastTile)
    {
        nextTile = grid[currentTile->getRow()][currentTile->getColumn() + 1];
        return true;
    }
    else if (direction == 3 && !currentTile->hasWall(Wall::SOUTH) 
    && currentTile->getRow() < 5 && !grid[currentTile->getRow() + 1][currentTile->getColumn()]->hasEvent(Event::FIRE)
    && grid[currentTile->getRow() + 1][currentTile->getColumn()] != lastTile)
    {
        nextTile = grid[currentTile->getRow() + 1][currentTile->getColumn()];
        return true;
    }
    else if (direction == 4 && !currentTile->hasWall(Wall::WEST) && currentTile->getColumn() > 0 
    && !grid[currentTile->getRow()][currentTile->getColumn() - 1]->hasEvent(Event::FIRE)
    && grid[currentTile->getRow()][currentTile->getColumn() - 1] != lastTile)
    {
        nextTile = grid[currentTile->getRow()][currentTile->getColumn() - 1];
        return true;
    }
    else
    {
        return false;
    }
}

bool Grid::checkForEvent(Event event)
{ 
    bool hasEvent = false;

    if (currentTile->hasEvent(event))
    {
        targetTile = currentTile;
        hasEvent = true;
    }    
    else if (!currentTile->hasWall(Wall::NORTH) && grid[currentTile->getRow() - 1][currentTile->getColumn()]->hasEvent(event))
    {
        targetTile = grid[currentTile->getRow() - 1][currentTile->getColumn()];
        hasEvent = true; 
    }
    else if (!currentTile->hasWall(Wall::EAST) && grid[currentTile->getRow()][currentTile->getColumn() + 1]->hasEvent(event))
    {
        targetTile = grid[currentTile->getRow()][currentTile->getColumn() + 1];
        hasEvent = true;
    }
    else if(!currentTile->hasWall(Wall::SOUTH) && grid[currentTile->getRow() + 1][currentTile->getColumn()]->hasEvent(event))
    {
        targetTile = grid[currentTile->getRow() + 1][currentTile->getColumn()];
        hasEvent = true;
    }
    else if (!currentTile->hasWall(Wall::WEST) && grid[currentTile->getRow()][currentTile->getColumn() - 1]->hasEvent(event))
    {
        targetTile = grid[currentTile->getRow()][currentTile->getColumn() - 1];
        hasEvent = true;
    }
    return hasEvent;
}

Tile*& Grid::getTile(int row, int column)
{
    return grid[row][column];
}

void Grid::bfsTo(Tile* destination)
{
    std::queue<Tile*> toVisit;  // BFS-kö för att hålla reda på vilka rutor som ska utforskas.
    std::unordered_map<Tile*, Tile*> parent;  // För att återskapa vägen från destination tillbaka till start.
    std::unordered_map<Tile*, bool> visited;  // Markera vilka rutor vi har besökt.
    pathToTarget.clear();  // Rensa eventuell tidigare beräknad väg.

    // Starta BFS från nuvarande ruta
    toVisit.push(currentTile);
    visited[currentTile] = true;
    parent[currentTile] = nullptr;

    bool found = false;  // Flagga för att hålla koll på om destinationen har hittats.

    int maxIterations = 1000; // Max antal iterationer för att undvika oändliga loopar
    int iterations = 0;

    while (!toVisit.empty() && !found && iterations < maxIterations)
    {
        iterations++;

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
                Tile* neighbor = getTile(newRow, newCol);  // Hämta grannen från rutnätet.

                // Kontrollera att grannen existerar
                if (!neighbor) continue;

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

    // Kontrollera om maxgränsen för iterationer nåddes
    if (iterations >= maxIterations) { return; }

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

void Grid::addWalls()
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

