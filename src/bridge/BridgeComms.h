#ifndef BRIDGECOMMS_H
#define BRIDGECOMMS_H

#include <Arduino.h>
#include "painlessmesh.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

class BridgeComms 
{
    private:
        //String nodeName = "Bridge";
        //namedMesh mesh;
        painlessMesh mesh;
        static void meshUpdate(void* pvParameters);
        static void meshBroadCastTask(void* pvParameters);
        static void serialWriteTask(void* pvParameters);
        static void serialReadTask(void* pvParameters);

    public:
        BridgeComms();
        ~BridgeComms();

        QueueHandle_t serialOutPutQueue;
        QueueHandle_t meshOutputQueue;

        void start();
        void enqueueMeshOutput(const String& msg);
        void enqueueSerialOutput(const String& msg);
};

#endif