#ifndef TILE_H_
#define TILE_H_

using namespace std;

enum class Event : int 
{
    SMOKE  = 0b0001,
    FIRE   = 0b0010,
    HAZMAT = 0b0100,
    VICTIM = 0b1000
};

enum class Wall : int 
{
    NORTH  = 0b0001,
    EAST   = 0b0010,
    SOUTH  = 0b0100,
    WEST   = 0b1000
};

class Tile
 {
    private: 

        int row;
        int column;
        int events; 
        int walls; 

    public:

        int firefighters;
        
        Tile(int row, int column);
        void addWall(Wall wall);
        bool hasWall(Wall wall) const;
        int getRow() const;
        int getColumn() const;
        void addEvent(Event e);
        void removeEvent(Event e);
        bool hasEvent(Event e);  
};

#endif
