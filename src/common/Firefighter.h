#ifndef FIREFIGHTER_H_
#define FIREFIGHTER_H_

#include "Tile.h"
#include <array>
#include <random>
#include <arduino.h>
#include <map>
#include "hardware_config.h"
#include <unordered_map>
#include <queue>
#include "Comms.h"


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
        Tile* grid[6][8]; 
        Tile* currentTile;
        Tile* targetTile; 
        Tile* lastTile;     
        Tile* exitTile;
        int missionTargetRow = 0;
        int missionTargetColumn = 0;
        int positionListCounter = 0;
        int tickCounter = 0;
        int nbrFirefighters;
        bool hasMission;
        bool pendingHelp = false;
        bool teamArrived;
        std::mt19937 gen;
        std::uniform_int_distribution<> dist;
        State state;  

        std::vector<String> tokenize(const String& expression);
        std::vector<uint32_t> teamMembers;
        std::vector<std::pair<uint32_t, float>> positionsList; // Map of node IDs to their positions
        std::vector<Tile*> pathToTarget; // Sparar den genererade vägen
        static void messageHandlerTask(void *pvParameters);  // FreeRTOS-task för att hantera meddelanden
        void handleMessage(uint32_t from, const char* msg);  // Metod för att hantera ett meddelande
        void handlePositions(uint32_t from, int row, int column);
        void handleHelpRequest(uint32_t from, int row, int column);
        void Tick(); 
        void searchForTarget();
        void moveToTarget();
        void extinguishFire();
        void extinguishSmoke();
        void moveHazmat();
        void rescuePerson();
        void move(const Tile* destination);
        void Die(int row, int column);
        void addWalls();
        void wait();
        void TeamArrived(); 
        void changeState();
        void bfsTo(Tile* destination);
        void startMessageHandler();  // Startar tasken
        bool atDeadEnd(); 
        bool checkForEvent(Tile* tile, Event event);
        bool tryParseInt(const String& str, int& outValue);

    public: 
        Comms comms;
        uint32_t leaderID;

        Firefighter();
        ~Firefighter(); 
        void startMission(); 
};

#endif  // FIREFIGHTER_H_
