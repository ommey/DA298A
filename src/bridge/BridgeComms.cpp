#include "BridgeComms.h"

BridgeComms::BridgeComms() : serialOutPutQueue(xQueueCreate(201, sizeof(char) * 256)), meshOutputQueue(xQueueCreate(201, sizeof(char) * 256)) 
{
    Serial.begin(115200);
    Serial.setTimeout(50);
    mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
    mesh.setName(nodeName);
  
    mesh.onReceive([this](String &from, String &msg) 
    {
        this->enqueueSerialOutput(msg);
    });        

    mesh.onChangedConnections([this]()
    {
        this->enqueueSerialOutput("Mesh message: Changed connection");
    });
}
    
void BridgeComms::meshUpdate(void *pvParameters)
{
    BridgeComms* comms = static_cast<BridgeComms*>(pvParameters);
    while (1) {
        comms->mesh.update();
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}

void BridgeComms::meshBroadCastTask(void *pvParameters)
{
    BridgeComms* comms = static_cast<BridgeComms*>(pvParameters);
    char msgChar[256];
    while (1) 
    {
    if (xQueueReceive(comms->meshOutputQueue, &msgChar, 10) == pdPASS) 
    {
        if (!comms->mesh.sendBroadcast(msgChar)) 
        {
        Serial.println("Failed to send broadcast");
        }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void BridgeComms::serialWriteTask(void *pvParameters)
{
    BridgeComms* comms = static_cast<BridgeComms*>(pvParameters);
    char msgChar[256];
    while (1) 
    {
        if (xQueueReceive(comms->serialOutPutQueue, &msgChar, 10) == pdPASS) 
        {
            if (msgChar != "") {
                try {
                    Serial.println(msgChar);
                }
                catch (...) {
                    
                }
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void BridgeComms::serialReadTask(void *pvParameters)
{
    BridgeComms* comms = static_cast<BridgeComms*>(pvParameters);
    while (1) 
    {
        if (Serial.available() > 0) 
        {
            String msg = Serial.readStringUntil('\n');
            comms->enqueueMeshOutput(msg);
        }
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}

BridgeComms::~BridgeComms()
{
    vQueueDelete(serialOutPutQueue);
    vQueueDelete(meshOutputQueue);
}

void BridgeComms::start()
{
    if (xTaskCreate(meshUpdate, "meshUpdate", 4096, this, 1, NULL) != pdPASS) {
        Serial.println("Failed to create meshUpdate task");
    }
    if (xTaskCreate(serialWriteTask, "serialWriteTask", 2048, this, 1, NULL) != pdPASS) {
        Serial.println("Failed to create serialWriteTask");
    }
    if (xTaskCreate(serialReadTask, "serialReadTask", 2048, this, 1, NULL) != pdPASS) {
        Serial.println("Failed to create serialReadTask");
    }
    if (xTaskCreate(meshBroadCastTask, "meshBroadCastTask", 4096, this, 1, NULL) != pdPASS) {
        Serial.println("Failed to create meshBroadCastTask");
    }
}

void BridgeComms::enqueueMeshOutput(const String &msg)
{
    if (msg != "") 
    {
        char msgChar[256];
        msg.toCharArray(msgChar, sizeof(msgChar));
        if (xQueueSend(meshOutputQueue, &msgChar, 10) != pdPASS) 
        {
            Serial.println("Failed to add to mesh queue");
        }
    }
}

void BridgeComms::enqueueSerialOutput(const String &msg)
{
    if (msg != "") 
    {
        char msgChar[256];
        msg.toCharArray(msgChar, sizeof(msgChar));
        if (xQueueSend(serialOutPutQueue, &msgChar, 10) != pdPASS) 
        {
            Serial.println("Failed to add to serial queue");
        }
    }
}
