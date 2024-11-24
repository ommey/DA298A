#include "Tile.h"
#include <array>
#include <random>
#include <queue>
#include <arduino.h>

#ifndef FIREFIGHTER_H_
#define FIREFIGHTER_H_

using namespace std;

enum class State
{
    SEARCHING,         
    MOVING,            
    PUTTING_OUT_FIRE,  
    PUTTING_OUT_SMOKE,
    MOVING_HAZMAT,     
    RESCUING_PERSON, 
    DEAD,
};

class Firefighter
{
    private:
        random_device rd;
        std::mt19937 gen;
        std::uniform_int_distribution<> row_dist;
        std::uniform_int_distribution<> col_dist;
        int id;
        bool firtsTick;
        State state; 
        Tile* lastTile;     
        Tile* targetTile; 
        Tile* exitTile; 

    public:

        Tile* currentTile;
        std::array<std::array<Tile, 8>, 6> grid;
        queue<String> messagesToBridge;  

        Firefighter();
        void setId(int id);
        int getId() const;    
        void Tick(); 
        void searchForTarget();
        void moveToTarget();
        void extinguishFire();
        void extinguishSmoke();
        void moveHazmat();
        void rescuePerson();
        void move(Tile& destination);
        bool ChangeState(Tile& tile);
        void Die();
        void addWalls();
        bool CheckSurroundingsForEvent();
};

#endif