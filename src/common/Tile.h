#ifndef TILE_H
#define TILE_H

using namespace std;

class Tile {
    private: 
        int x;
        int y;
        int events = 0b0000;
        int walls = 0b0000;

    public:
        Tile();
        Tile(int x, int y);
        void setWalls(int walls);
        int getWalls() const;
        void setCoordinates(int x, int y);
        int getX() const;
        int getY() const;
        void setEvents(int events);
        int getEvents() const;
};

#endif TILE_H