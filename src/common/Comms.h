#ifndef COMMS_H
#define COMMS_H

#include "painlessMesh.h"
#include "Firefighter.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

class Comms 
{
    private:
        Firefighter* firefighter;
        painlessMesh mesh;

        static void meshUpdate(void* pvParameters);
        static void meshWriteTask(void* pvParameters);

    public:
        QueueHandle_t meshOutputQueue;

        Comms(Firefighter* firefighter);

        ~Comms();

        void start();

        void enqueueMeshOutput(const Message& msg);
};

#endif