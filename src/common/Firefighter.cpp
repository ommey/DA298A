#include "Firefighter.h"
#include "Tile.h"

    Firefighter::Firefighter(int id, Tile currentTile, const std::array<std::array<Tile, 6>, 8>& grid) : id(id), currentTile(currentTile), lastTile(currentTile), grid(grid), hasTarget(false) {}   
    Firefighter::Firefighter(int id) : id(id) {}
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
        void Firefighter::setCarryingPerson(const bool carryingPerson){
            this->carryingPerson = carryingPerson;
        }
        bool Firefighter::getCarryingPerson() const {
            return carryingPerson;
        }
        void Firefighter::setCarryingHazmat(const bool carryingHazmat){
            this->carryingHazmat = carryingHazmat;
        }
        bool Firefighter::getCarryingHazmat() const {
            return carryingHazmat;
        }
        void Firefighter::move(int x, int y) {
            setLastTile(currentTile);
            Tile newTile = grid[x][y];
            currentTile = newTile;
        }
        int Firefighter::handleSmokeFire(int x, int y) {
            int event = grid[x][y].getEvents();
            if (event == 0b0001) {
                grid[x][y].setEvents(0b0000);
            } else if (event == 0b0010) {
                grid[x][y].setEvents(0b0001);
            } 
            return event;
        }
        void Firefighter::addEvent(int x, int y, int event) {
            if (event == 0b1000) {
                grid[x][y].setEvents(grid[x][y].getEvents() | (1 << 3));
            } else if (event == 0b0100) {
                grid[x][y].setEvents(grid[x][y].getEvents() | (1 << 2));
            } else if (event == 0b0010) {
                grid[x][y].setEvents(grid[x][y].getEvents() | (1 << 1));
            } else {
                grid[x][y].setEvents(grid[x][y].getEvents() | 1);
            }
        }
        void Firefighter::removeEvent(int x, int y, int event) {
            if (event == 0b1000) {
                grid[x][y].setEvents(grid[x][y].getEvents() & ~(1 << 3));
            } else if (event == 0b0100) {
                grid[x][y].setEvents(grid[x][y].getEvents() & ~(1 << 2));
            } else if (event == 0b0010) {
                grid[x][y].setEvents(grid[x][y].getEvents() & ~(1 << 1));
            } else {
                grid[x][y].setEvents(grid[x][y].getEvents() & ~1);
            }
        }