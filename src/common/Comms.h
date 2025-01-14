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

        static void serialWriteTask(void* pvParameters);
        static void serialReadTask(void* pvParameters);

        const unsigned long DEBOUNCE_DELAY = 1000; // Debounce delay in milliseconds

    public:

        Comms(Firefighter* firefighter);
        ~Comms();

        QueueHandle_t meshOutputQueue;
        QueueHandle_t serialOutPutQueue;

        void start();
        void enqueueMeshOutput(const Message& msg);
        void enqueueSerialOutput(const String& msg);

        static Comms* instance;
};

#endif