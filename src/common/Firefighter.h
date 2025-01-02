#ifndef FIREFIGHTER_H_
#define FIREFIGHTER_H_

#include "Grid.h"
#include <random>
#include "hardware_config.h"

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

struct Message
{
    uint32_t from;   
    char message[100]; 

    Message() : from(0) { message[0] = '\0'; }

    Message(uint32_t fromID, const char* msg) : from(fromID)
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
        int id;
        State state; 
        bool teamArrived;
        uint32_t bridgeName = 533097877;
        QueueHandle_t* serialOutputQueue;
        QueueHandle_t* meshOutPutQueue;

    public:
        Grid grid; 
        bool hasMission;
        int nbrFirefighters;
        uint32_t leaderID; 
        vector<uint32_t> teamMembers;
        bool pendingHelp = false;
        int tickCounter = 0;

        queue<String> messagesToBridge; // meddelanden som ska skickas till bridge
        queue<String> messagesToBroadcast; // meddelanden som ska skickas till alla noder (inte till bridge)
        queue<pair<uint32_t, String>> messagesToNode; // meddelanden som ska skickas till en specific nod

        vector<pair<uint32_t, float>> positionsList; // Map of node IDs to their positions
        vector<Tile*> pathToTarget; // Sparar den genererade vägen

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
        void startMission(int row, int column);
        void changeState();
        void handleMessage(uint32_t from, String msg);
        void registerSerialOutput(QueueHandle_t* serialOutputQueue);
        void registerMeshOutput(QueueHandle_t* meshOutPutQueue);
        void enqueueMeshOutput(const Message& msg);
        void enqueueSerialOutput(const String& msg);
};

#endif  // FIREFIGHTER_H_
