#include "Comms.h"

Comms::Comms(Firefighter* firefighter) : firefighter(firefighter), meshOutputQueue(xQueueCreate(100, sizeof(Message))) 
{
    Serial.begin(115200);
    Serial.setTimeout(50);
    mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);

    if (meshOutputQueue == nullptr) 
    {
        Serial.println("Failed to create one or more queues. Heap memory may be insufficient.");
        while (true) {}
    }

    mesh.onReceive([this](uint32_t from, String &msg) 
    {    
        this->firefighter->handleMessage(from, msg);
    });
    
    mesh.onChangedConnections([this]() 
    {
        this->firefighter->nbrExpectedAnswers = mesh.getNodeList().size()-2;
    });

    mesh.onDroppedConnection([this](size_t nodeId) 
    {
    });
}
    
void Comms::meshUpdate(void *pvParameters)
{
    Comms* comms = static_cast<Comms*>(pvParameters);
    while (1) 
    {
        comms->mesh.update();
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}

void Comms::meshWriteTask(void *pvParameters)
{
    Comms* comms = static_cast<Comms*>(pvParameters);
    Message message; 
    while (1) 
    {        
        if (xQueueReceive(comms->meshOutputQueue, &message, 10) == pdPASS) 
        {
            if (message.from == 0 && message.sendToBridge)
            {
                if (!comms->mesh.sendBroadcast(message.message)) 
                {
                    Serial.println("Failed to send broadcast");
                }
            } 
            if (message.from == 0 && !message.sendToBridge)
            {
                for (uint32_t node : comms->mesh.getNodeList())   
                {
                    if (node != comms->firefighter->bridgeName) 
                    {
                        if (!comms->mesh.sendSingle(node, message.message)) 
                        {
                            Serial.println("Failed to send singel");
                        }
                    }
                }
            }            
            else 
            {
                if (!comms->mesh.sendSingle(message.from, message.message)) 
                {
                    Serial.println("Failed to send singel");
                }
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

Comms::~Comms()
{
    vQueueDelete(meshOutputQueue);
}

void Comms::start()
{
    if (xTaskCreate(meshUpdate, "meshUpdate", 8192, this, 1, NULL) != pdPASS) {
        Serial.println("Failed to create meshUpdate task");
    }
    if (xTaskCreate(meshWriteTask, "meshBroadCastTask", 8192, this, 1, NULL) != pdPASS) {
        Serial.println("Failed to create meshBroadCastTask");
    }
}

void Comms::enqueueMeshOutput(const Message &msg)
{
    if (msg.message != "") 
    {
        if (xQueueSend(meshOutputQueue, &msg, 10) != pdPASS) 
        {
            Serial.println("Failed to add to mesh queue");
        }
    }
}
