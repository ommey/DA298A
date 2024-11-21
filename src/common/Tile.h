#ifndef TILE_H_
#define TILE_H_

using namespace std;

class Tile {
    private: 
        int x;
        int y;
        int events = 0b0000; // person, material, fire, smoke
        int walls = 0b0000; // west, south, east, north

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

#endif // TILE_H_
