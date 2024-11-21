#include "Firefighter.h"
#include "Tile.h"

    Firefighter::Firefighter(int id, Tile currentTile, const std::array<std::array<Tile, 6>, 8>& grid) : id(id), currentTile(currentTile), lastTile(currentTile), grid(grid), hasTarget(false) {}   
    Firefighter::Firefighter() : id(0) {}
    Firefighter::Firefighter(int id) : id(id) {}

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
        void Firefighter::setTargetTile(const Tile& targetTile){
            this->targetTile = targetTile;
        }
        Tile Firefighter::getTargetTile(){
            return targetTile;
        }
        void Firefighter::setHasTarget(const bool hasTarget){
            this->hasTarget = hasTarget;
        }
        bool Firefighter::getHasTarget() const {
            return hasTarget;
        }
        void Firefighter::move(int x, int y) {
            setLastTile(currentTile);
            Tile newTile = grid[x][y];
            currentTile = newTile;
        }
        int Firefighter::handleSmokeFire(int x, int y) {
            int events = grid[x][y].getEvents();
            if (events == 0b0001) {
                grid[x][y].setEvents(0b0000);
            } else if (events == 0b0010) {
                grid[x][y].setEvents(0b0001);
            } 
            return grid[x][y].getEvents();
        }