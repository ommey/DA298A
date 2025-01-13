#ifndef GRID_H_
#define GRID_H_

#include "Tile.h"
#include <arduino.h>
#include <unordered_map>
#include <queue>

using namespace std;

class Grid
{
    private:
        Tile* grid[6][8]; 
        void addWalls();

    public: 
        Tile* currentTile;
        Tile* targetTile; 
        Tile* lastTile;     
        Tile* exitTile;
        std::vector<Tile*> pathToTarget; // Sparar den genererade vägen

        Grid(); 
        ~Grid();
        void update(String event, int row, int column);
        Tile*& getTile(int row, int column);
        bool checkForEvent(Event event);
        bool getNextTile(int direction, Tile*& nextTile);
        bool atDeadEnd();
        void bfsTo(Tile* destination);        
};

#endif 