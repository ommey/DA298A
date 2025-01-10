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
    VICTIM
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
        random_device rd;
        mt19937 gen;
        uniform_int_distribution<> dist;
        State state; 
        bool teamArrived;
        //QueueHandle_t* serialOutputQueue;
        QueueHandle_t* meshOutPutQueue;
        int missionTargetRow = 0;
        int missionTargetColumn = 0;
        int positionListCounter = 0;

        bool tryParseInt(const String& str, int& outValue);  
        vector<String> tokenize(const String& expression);

    public:
        Grid grid; 
        bool hasMission;
        int nbrFirefighters = 1;
        uint32_t leaderID = 0; 
        vector<uint32_t> teamMembers;
        bool pendingHelp;
        int tickCounter = 0;
        int nbrExpectedAnswers = 0;
        uint32_t bridgeName = 533097877;

        vector<pair<uint32_t, float>> positionsList; // Map of node IDs to their positions

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
        void Die(int row, int column);
        void wait();
        void TeamArrived();
        void startMission();
        void changeState();
        void handleMessage(uint32_t from, String msg);
        //void registerSerialOutput(QueueHandle_t* serialOutputQueue);
        void registerMeshOutput(QueueHandle_t* meshOutPutQueue);
        void enqueueMeshOutput(const Message& msg);
        //void enqueueSerialOutput(const String& msg);
        void handleHelpRequest(uint32_t from, int row, int column);
        void handlePositions(uint32_t from, int row, int column);
};

#endif  // FIREFIGHTER_H_
