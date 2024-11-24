#include "Tile.h"
#include <arduino.h>
   
    Tile::Tile(int row, int column)
    {
        this->row = row;
        this->column = column;
        this->firefighters = 0;
    }

    void Tile::addWall(Wall wall) 
    {
        walls |= static_cast<int>(wall);
    }

    bool Tile::hasWall(Wall wall) const 
    {
        return (walls & static_cast<int>(wall)) != 0;
    }

    int Tile::getRow() const 
    {
        return row;
    }

    int Tile::getColumn() const 
    {
        return column;
    }

    void Tile::addEvent(Event e) 
    {
        events |= static_cast<int>(e);
    }

    void Tile::removeEvent(Event e)
    {
        events &= ~static_cast<int>(e);
    }

    bool Tile::hasEvent(Event e) 
    {
        return (events & static_cast<int>(e)) != 0;
    }
