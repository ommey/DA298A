#include "Tile.h"
#include <arduino.h>
   
    Tile::Tile(int row, int column)
    {
        this->row = row;
        this->column = column;
        this->events = false;
        this->walls = false;
    }

    void Tile::addWall(Wall wall) 
    {
        this->walls |= static_cast<int>(wall);
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
        this->events |= static_cast<int>(e);
    }

    void Tile::removeEvent(Event e)
    {
        this->events &= ~static_cast<int>(e);
    }
    bool Tile::hasEvent(Event e) 
    {
        return (events & static_cast<int>(e)) != 0;
    }
