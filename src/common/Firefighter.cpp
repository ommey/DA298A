#include "Firefighter.h"
#include "Tile.h"

    Firefighter::Firefighter(int id, Tile currentTile, const std::array<std::array<Tile, 6>, 8>& grid) : id(id), currentTile(currentTile), lastTile(currentTile), grid(grid) {}   
    Firefighter::Firefighter() : id(0) {}

        void Firefighter::setId(int id){
            this->id = id;
        }
        int Firefighter::getId() const {
            return id;
        }
        void Firefighter::setCurrentTile(const Tile& currenttile) {
            this->currentTile = currentTile;
        }
        Tile Firefighter::getCurrentTile() {
            return currentTile;
        }
        void Firefighter::setLastTile(const Tile& lastTile) {
            this->lastTile = lastTile;
        }
        Tile Firefighter::getLastTile(){
            return lastTile;
        }
        void Firefighter::setGrid(const std::array<std::array<Tile, 6>, 8>& grid) {
            this->grid = grid;
        }
        std::array<std::array<Tile, 6>, 8>& Firefighter::getGrid() {
            return grid;
        }
        void Firefighter::move(int x, int y) {
            setLastTile(currentTile);
            Tile newTile = grid[x][y];
            currentTile = newTile;
        }