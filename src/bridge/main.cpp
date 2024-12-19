#include <Arduino.h>
#include "painlessMesh.h"
#include "spi.h"

#define   MESH_SSID       "meshNetwork"
#define   MESH_PASSWORD   "meshPassword"
#define   MESH_PORT       5555

painlessMesh mesh; //variant på painlessMesh som kan skicka meddelanden till specifika noder

class Comms 
{
    private:

        static painlessMesh mesh;
        static QueueHandle_t serialOutPutQueue;
        static QueueHandle_t meshOutputQueue;

        static void meshUpdate(void *pvParameters)
        {
        while(1) 
            {
                mesh.update();
                vTaskDelay(30 / portTICK_PERIOD_MS);
            }
        }

        static void meshBroadCastTask(void *p)
        {
            while(1)
            {
                char msgChar[256];
                if (xQueueReceive(meshOutputQueue, &msgChar, 10) == pdPASS) 
                {   
                    if (!mesh.sendBroadcast(msgChar)) 
                    {
                        Serial.println("Failed to send broadcast");
                    }
                }
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }
        }

        static void serialWriteTask(void *p)
        {
            while(1)
            {
                char msgChar[256];
                if (xQueueReceive(serialOutPutQueue, &msgChar, 10) == pdPASS) 
                {   
                    Serial.println(msgChar); 
                }
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }
        }

        static void serialReadTask(void *p)
        {
            while(1)
            {
                //ska läsa strängar från serial och stoppa in i broadcast mesh grajen
                if(Serial.available()>0)
                {
                    String msg = Serial.readStringUntil('\n');
                    serialOutPut(msg); // för debug
                    meshOutPut(msg); // pang på rödbetan
                }
                vTaskDelay(30 / portTICK_PERIOD_MS);
            }
        }

    public:

        static void meshOutPut(const String& msg)
        {
            if (msg != "") 
            {
                char msgChar[256];
                msg.toCharArray(msgChar, sizeof(msgChar));
                if (!xQueueSend(meshOutputQueue, &msgChar, 10) == pdPASS) 
                {
                    Serial.println("Failed to add to mesh-queue");
                }
            }
        }
            
        static void serialOutPut(const String& msg)
        {
            if (msg != "") 
            {
                char msgChar[256];
                msg.toCharArray(msgChar, sizeof(msgChar));
                if (!xQueueSend(serialOutPutQueue, &msgChar, 10) == pdPASS) 
                {
                    Serial.println("Failed to add to serial queue");
                }
            }
        }

        Comms() 
        {
            Serial.begin(115200);
            Serial.setTimeout(50);
            mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
            mesh.onReceive([](uint32_t from, String &msg) {
                serialOutPut(msg);
            });
            serialOutPutQueue = xQueueCreate(100, sizeof(char)* 256);
            meshOutputQueue = xQueueCreate(100, sizeof(char)* 256);
            xTaskCreate(meshUpdate, "meshUpdate", 5000, NULL, 1, NULL);
            xTaskCreate(serialWriteTask, "serialWriteTask", 5000, NULL, 1, NULL);
            xTaskCreate(serialReadTask, "serialReadTask", 5000, NULL, 1, NULL);
            xTaskCreate(meshBroadCastTask, "meshBroadCastTask", 5000, NULL, 1, NULL);
        }
};

QueueHandle_t Comms::serialOutPutQueue = nullptr;
QueueHandle_t Comms::meshOutputQueue = nullptr;
painlessMesh Comms::mesh;


void setup() 
{
    delay(1000);
    Comms comms;
    comms.serialOutPut("Just started");
    comms.meshOutPut("Just started");
}

void loop()
{}  // inget görs här, aktiviteter sköts i freeRTOS tasks
