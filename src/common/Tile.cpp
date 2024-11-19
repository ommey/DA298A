#include "Tile.h"
   
    Tile::Tile(int x, int y) : x(x), y(y), walls(0b0000), events(0b0000) {}

    Tile::Tile() :x(0), y(0), walls(0b0000), events(0b0000) {}

        void Tile::setWalls(int walls) {
            this->walls = walls;
        }

        int Tile::getWalls() const {
            return walls;
        }

        void Tile::setCoordinates(int x, int y) {
            this->x = x;
            this->y = y;
        }
        int Tile::getX() const {
            return x;
        }
        int Tile::getY() const {
            return y;
        }

        void Tile::setEvents(int events) {
            this->events = events;
        }
        int Tile::getEvents() const {
            return this->events;
        } 
