#ifndef FIREFIGHTER_H_
#define FIREFIGHTER_H_

#include "Tile.h"
#include <random>
#include <queue>
#include <arduino.h>
#include <map>

using namespace std;

enum class State
{
    SEARCHING, 
    MOVING_TO_TARGET,        
    WAITING,            
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
        std::uniform_int_distribution<> dist;
        int id;
        bool hasMission; 
        State state; 
        Tile* lastTile;     
        Tile* targetTile; 
        Tile* exitTile; 

    public:
        // Grid of pointers to Tile objects
        Tile* grid[6][8]; // Dynamically allocated grid of pointers
        Tile* currentTile;
        queue<String> messagesToBridge;
        queue<String> messagesToNode;
        std::map<String, bool> team;  

        Firefighter();
        ~Firefighter();  // Destructor for cleaning up dynamic memory

        void setId(int id);
        int getId() const;    
        void Tick(); 
        void searchForTarget();
        void moveToTarget();
        void extinguishFire();
        void extinguishSmoke();
        void moveHazmat();
        void rescuePerson();
        void move(const Tile* destination);
        bool ChangeState(Tile* tile);
        void Die();
        void addWalls();
        void wait(); 
        void startMission(int row, int column);
        bool teamArrived();
        bool atDeadEnd(); 
        bool checkForEvent(Tile* tile, Event event); 
        void changeState();

        // Helper function to allocate memory for grid
        void initializeGrid();
        void cleanupGrid();

        void printGrid();
};

#endif  // FIREFIGHTER_H_
