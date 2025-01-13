#ifndef FIREFIGHTER_H_
#define FIREFIGHTER_H_

#include "Grid.h"
#include <random>
#include "hardware_config.h"
#include <sstream>

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
    VICTIM,
};

struct Message
{
    uint32_t from;   
    char message[50]; 
    bool sendToBridge;

    Message() : from(0) { message[0] = '\0'; }

    Message(uint32_t fromID, const char* msg, bool sendToBridge = false) : from(fromID), sendToBridge(sendToBridge)
    {
        strncpy(message, msg, sizeof(message) - 1);
        message[sizeof(message) - 1] = '\0';
    }
};

class Firefighter
{
    private:
        State state; 
        mt19937 gen;
        random_device rd;
        uniform_int_distribution<> dist;
        QueueHandle_t* meshOutPutQueue;
        vector<uint32_t> teamMembers;
        bool hasMission;
        bool teamArrived;
        int missionTargetRow = 0;
        int missionTargetColumn = 0;
        int positionListCounter = 0;
        int nbrFirefighters = 0;
    
        void Tick(); 
        void searchForTarget();
        void moveToTarget();
        void extinguishFire();
        void extinguishSmoke();
        void moveHazmat();
        void rescuePerson();
        void move(const Tile* destination);
        void Die(int row, int column);
        void wait();
        void TeamArrived();
        void changeState();
        void enqueueMeshOutput(const Message& msg);
        void handleHelpRequest(uint32_t from, int row, int column);
        void handlePositions(uint32_t from, int row, int column);
        void reset();
        bool ChangeState(Tile* tile);
        bool tryParseInt(const String& str, int& outValue);  
        vector<String> tokenize(const String& expression);

    public: 
        Grid grid;
        uint32_t bridgeName = 533097877;
        uint32_t leaderID = 0; 
        vector<pair<uint32_t, float>> positionsList; // Map of node IDs to their positions
        int nbrExpectedAnswers = 0;
        int tickCounter = 0;
        bool pendingHelp;

        Firefighter();
        ~Firefighter();
        void startMission();
        void registerMeshOutput(QueueHandle_t* meshOutPutQueue);
        void handleMessage(uint32_t from, String msg);

};

#endif  // FIREFIGHTER_H_
