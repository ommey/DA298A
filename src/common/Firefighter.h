#ifndef FIREFIGHTER_H_
#define FIREFIGHTER_H_

#include "Grid.h"
#include "Comms.h"
#include <array>
#include <random>
#include <queue>
#include <arduino.h>
#include <map>
#include "hardware_config.h"
#include <unordered_map>


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
        State state;    
        bool teamArrived;
        //void handleMessage(uint32_t from, const char* msg);
        //void messageHandlerTask(void *pvParameters);
        vector<String> tokenize(const String& expression);
        bool tryParseInt(const String& str, int& outValue);
        void handlePositions(uint32_t from, int row, int column);
        void handleHelpRequest(uint32_t from, int row, int column);
        static void messageHandlerTask(void *pvParameters);
        void handleMessage(uint32_t from, const char* msg);

    public:
        // Grid of pointers to Tile objects
        Grid grid;
        Comms comms;
        bool hasMission;
        int nbrFirefighters;
        uint32_t leaderID; 
        std::vector<uint32_t> teamMembers;
        bool pendingHelp = false;
        int tickCounter = 0;
        int missionTargetRow;
        int missionTargetColumn;
        int positionListCounter;

        std::vector<std::pair<uint32_t, float>> positionsList; // Map of node IDs to their positions
        std::vector<Tile*> pathToTarget; // Sparar den genererade vägen

        Firefighter();
       // ~Firefighter();  // Destructor for cleaning up dynamic memory

 
        void Tick(); 
        void searchForTarget();
        void moveToTarget();
        void extinguishFire();
        void extinguishSmoke();
        void moveHazmat();
        void rescuePerson();
        void move(const Tile* destination);
        bool ChangeState(Tile* tile);
        void Die(int row, int column);
        void wait();
        void TeamArrived();
        void startMission(int row, int column);
        void changeState();
        void startMission();

        
};

#endif  // FIREFIGHTER_H_
