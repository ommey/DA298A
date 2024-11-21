#include "Tile.h"
#include <array>
#ifndef FIREFIGHTER_H_
#define FIREFIGHTER_H_

using namespace std;

class Firefighter {
    private:
        int id;
        Tile currentTile;
        Tile lastTile;
        std::array<std::array<Tile, 6>, 8> grid;
        Tile targetTile;
        bool hasTarget = false;

    public:
        Firefighter(int id, Tile currentTile, const std::array<std::array<Tile, 6>, 8>& grid);
        Firefighter(int id);
        Firefighter();
        void setId(int id);
        int getId() const;
        void setCurrentTile(const Tile& currentTile);
        Tile getCurrentTile();
        void setLastTile(const Tile& lastTile);
        Tile getLastTile();
        void setGrid(const std::array<std::array<Tile, 6>, 8>& grid);
        std::array<std::array<Tile, 6>, 8>& getGrid(); 
        void move(int x, int y);
        int handleSmokeFire(int x, int y);
        void setTargetTile(const Tile& targetTile);
        Tile getTargetTile();
        void setHasTarget(const bool hasTarget);
        bool getHasTarget() const;
};

#endif