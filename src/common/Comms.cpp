#include "Comms.h"

Comms::Comms(Firefighter *firefighter) : firefighter(firefighter), meshOutputQueue(xQueueCreate(100, sizeof(Message))), serialOutPutQueue(xQueueCreate(100, sizeof(char) * 50)) 
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
        this->enqueueSerialOutput("Received message from: " + String(from) + " with content: " + msg);
    });

    mesh.onChangedConnections([this]() 
    {
        this->firefighter->nbrExpectedAnswers = mesh.getNodeList().size()-1;
    });

    mesh.onDroppedConnection([this](size_t nodeId) { });
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
                if (!comms->mesh.sendBroadcast(message.message)) { Serial.println("Failed to send broadcast"); }
            }
            if (message.from == 0 && !message.sendToBridge) 
            {
                for (uint32_t node : comms->mesh.getNodeList()) 
                {
                    if (node != comms->firefighter->bridgeName) 
                    {
                        if (!comms->mesh.sendSingle(node, message.message)) { Serial.println("Failed to send singel"); }
                    }
                }
            }
            else 
            {
                if (!comms->mesh.sendSingle(message.from, message.message)) { Serial.println("Failed to send singel"); }
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void Comms::serialWriteTask(void *pvParameters) 
{
    Comms* comms = static_cast<Comms*>(pvParameters);
    char msgChar[256];
    while (1) 
    {
        if (xQueueReceive(comms->serialOutPutQueue, &msgChar, 10) == pdPASS) 
        {
            if (msgChar != "") 
            {
                try { Serial.println(msgChar); }
                catch (...) { }
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void Comms::serialReadTask(void *pvParameters) 
{
    Comms* comms = static_cast<Comms*>(pvParameters);
    while (1) 
    {
        if (Serial.available() > 0) 
        {
            String msg = Serial.readStringUntil('\n');
            if(msg == "Reset") 
            {
                comms->enqueueSerialOutput("Reset was called");
                //comms->firefighter = new Firefighter();
                string messageContent = "RemoveFirefighter " + to_string(comms->firefighter->grid.currentTile->getRow()) + " " + to_string(comms->firefighter->grid.currentTile->getColumn());
                comms->enqueueMeshOutput(Message(comms->firefighter->bridgeName, messageContent.c_str()));
                vTaskDelay(5000 / portTICK_PERIOD_MS);
                ESP.restart();
            }
        }
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}

Comms::~Comms() { vQueueDelete(meshOutputQueue); }

void Comms::start() 
{
    if (xTaskCreate(meshUpdate, "meshUpdate", 8192, this, 1, NULL) != pdPASS) { Serial.println("Failed to create meshUpdate task"); }
    if (xTaskCreate(meshWriteTask, "meshBroadCastTask", 8192, this, 1, NULL) != pdPASS) { Serial.println("Failed to create meshBroadCastTask"); }
    if (xTaskCreate(serialWriteTask, "serialWriteTask", 2048, this, 1, NULL) != pdPASS) { Serial.println("Failed to create serialWriteTask"); }
    if (xTaskCreate(serialReadTask, "serialReadTask", 2048, this, 1, NULL) != pdPASS) { Serial.println("Failed to create serialReadTask"); }
}

void Comms::enqueueMeshOutput(const Message &msg) 
{
    if (msg.message != "") 
    {
        if (xQueueSend(meshOutputQueue, &msg, 10) != pdPASS) { Serial.println("Failed to add to mesh queue"); }
    }
}

void Comms::enqueueSerialOutput(const String &msg) 
{
    if (msg != "") 
    {
        char msgChar[256];
        msg.toCharArray(msgChar, sizeof(msgChar));
        if (xQueueSend(serialOutPutQueue, &msgChar, 10) != pdPASS) { Serial.println("Failed to add to serial queue"); }
    }
}
