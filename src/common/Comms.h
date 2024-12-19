#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
#include "painlessMesh.h"

// Definiera nätverkskonfigurationer
#define MESH_SSID       "meshNetwork"
#define MESH_PASSWORD   "meshPassword"
#define MESH_PORT       5555
//#define BRIDGE_NAME     244620401
#define BRIDGE_NAME     533097877

// Meddelandestruktur
struct Message 
{
    uint32_t to;
    char msg[256];

    Message(uint32_t from, const char* message);
    Message();
};

// Kommunikationsklassen
class Comms 
{
private:
    static painlessMesh mesh;
    static QueueHandle_t serialOutPutQueue;
    static QueueHandle_t meshOutputQueue;

    // FreeRTOS task-funktioner
    static void meshUpdate(void* pvParameters);
    static void meshWriteTask(void* p);
    static void serialWriteTask(void* p);
    static void serialReadTask(void* p);
    static void incomingMessagesPush(uint32_t from, const String& msg);

public:
    static void meshPush(const String& msg, uint32_t nodeid = 0);
    static void serialPush(const String& msg);
    static QueueHandle_t incomingMessages;
    static std::__cxx11::list<uint32_t> getNodeList();

    Comms();
};

#endif // COMMS_H
