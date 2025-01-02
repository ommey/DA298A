#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
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

    public:
        QueueHandle_t serialOutPutQueue;
        QueueHandle_t meshOutputQueue;

        QueueHandle_t getSerialOutPutQueue();

        Comms(Firefighter* firefighter);

        ~Comms();

        void start();

        void enqueueMeshOutput(const Message& msg);

        void enqueueSerialOutput(const String& msg);
};

#endif