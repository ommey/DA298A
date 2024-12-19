#include "Comms.h"

// Initialisering av statiska medlemmar
QueueHandle_t Comms::serialOutPutQueue = nullptr;
QueueHandle_t Comms::meshOutputQueue = nullptr;
QueueHandle_t Comms::incomingMessages = nullptr;
painlessMesh Comms::mesh;

// Konstruktorer för Message
Message::Message(uint32_t from, const char* message) : to(from) 
{
    strncpy(msg, message, sizeof(msg) - 1);
    msg[sizeof(msg) - 1] = '\0'; // Säkerställ null-terminering
}

Message::Message() : to(0) 
{
    msg[0] = '\0';
}

Comms::Comms()
{
    Serial.begin(115200);
    Serial.setTimeout(50);
    
    mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);

    mesh.onReceive([](uint32_t from, String& msg) {
        meshPush(msg);
    });

    serialOutPutQueue = xQueueCreate(100, sizeof(char[256]));
    meshOutputQueue = xQueueCreate(100, sizeof(Message));
    incomingMessages = xQueueCreate(100, sizeof(Message));

    xTaskCreate(meshUpdate, "meshUpdate", 5000, NULL, 1, NULL);
    xTaskCreate(serialWriteTask, "serialWriteTask", 5000, NULL, 1, NULL);
    xTaskCreate(serialReadTask, "serialReadTask", 5000, NULL, 1, NULL);
    xTaskCreate(meshWriteTask, "meshWriteTask", 5000, NULL, 1, NULL);
}

void Comms::meshUpdate(void* pvParameters)
{
    while (1) 
    {
        mesh.update();
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}

void Comms::meshWriteTask(void* p)
{
    while (1)
    {
        Message msg;
        if (xQueueReceive(meshOutputQueue, &msg, portMAX_DELAY) == pdPASS) 
        {
            if (msg.to == 0) 
            {
                for (uint32_t node : mesh.getNodeList()) 
                {
                    if (node != BRIDGE_NAME) 
                    {
                        if (!mesh.sendSingle(node, msg.msg)) 
                        {
                            Serial.println("Failed to send message");
                        }
                    }
                }
            }             
            else if (msg.to == 1)
            {
                if (!mesh.sendBroadcast(msg.msg)) 
                {
                    Serial.println("Failed to send broadcast");
                }
            }
            else if (msg.to == mesh.getNodeId()) 
            {
                Serial.println("Message to me: " + String(msg.msg));
            }
            else 
            {
                if (!mesh.sendSingle(msg.to, msg.msg)) 
                {
                    Serial.println("Failed to send message");
                }
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void Comms::serialWriteTask(void* p)
{
    while (1)
    {
        char msgChar[256];
        if (xQueueReceive(serialOutPutQueue, &msgChar, portMAX_DELAY) == pdPASS) 
        {
            Serial.println(msgChar); 
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void Comms::serialReadTask(void* p)
{
    while (1)
    {
        if (Serial.available() > 0)
        {
            String msg = Serial.readStringUntil('\n');
            serialPush(msg); // För debug
            meshPush(msg);   // Broadcast
        }
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}

void Comms::meshPush(const String& msg, uint32_t nodeid)
{
    if (!msg.isEmpty()) 
    {
        char msgChar[256];
        msg.toCharArray(msgChar, sizeof(msgChar));
        Message message(nodeid, msgChar);

        if (xQueueSend(meshOutputQueue, &message, portMAX_DELAY) != pdPASS) 
        {
            Serial.println("Failed to add to mesh-queue");
        }
    }
}

void Comms::serialPush(const String& msg)
{
    if (!msg.isEmpty()) 
    {
        char msgChar[256];
        msg.toCharArray(msgChar, sizeof(msgChar));

        if (xQueueSend(serialOutPutQueue, &msgChar, portMAX_DELAY) != pdPASS) 
        {
            Serial.println("Failed to add to serial queue");
        }
    }
}

void Comms::incomingMessagesPush(uint32_t from, const String& msg)
{
    if (!msg.isEmpty()) 
    {
        char msgChar[256];
        msg.toCharArray(msgChar, sizeof(msgChar));
        Message message(from, msgChar);

        if (xQueueSend(incomingMessages, &message, portMAX_DELAY) != pdPASS) 
        {
            Serial.println("Failed to add to mesh-queue");
        }
    }
}

std::__cxx11::list<uint32_t> Comms::getNodeList()
{
    return mesh.getNodeList();
}