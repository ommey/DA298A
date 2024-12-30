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

class Firefighter
{
    private:
        random_device rd;
        std::mt19937 gen;
        std::uniform_int_distribution<> dist;
        int id;
        State state; 
        bool teamArrived;

    public:
        Grid grid; 
        bool hasMission;
        int nbrFirefighters;
        uint32_t leaderID; 
        std::vector<uint32_t> teamMembers;
        bool pendingHelp = false;
        int tickCounter = 0;

        queue<String> messagesToBridge; // meddelanden som ska skickas till bridge
        queue<String> messagesToBroadcast; // meddelanden som ska skickas till alla noder (inte till bridge)
        queue<std::pair<uint32_t, String>> messagesToNode; // meddelanden som ska skickas till en specific nod

        std::vector<std::pair<uint32_t, float>> positionsList; // Map of node IDs to their positions
        std::vector<Tile*> pathToTarget; // Sparar den genererade vägen

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
};

#endif  // FIREFIGHTER_H_
