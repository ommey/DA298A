#include "hardware_config.h"
#include <Arduino.h>

void setup() {
    // Initiera seriell kommunikation för debug
    Serial.begin(115200);

    // Ställ in knapparna som ingångar
    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);
    pinMode(BUTTON_3, INPUT_PULLUP);
}

void loop() {
    // Läs av knappstatus
    int button1State = digitalRead(BUTTON_1);
    int button2State = digitalRead(BUTTON_2);
    int button3State = digitalRead(BUTTON_3);

    // Kontrollera om knapparna är tryckta (LOW betyder tryckt om pull-up används)
    if (button1State == LOW) {
        Serial.println("Button 1 pressed");
    }
    if (button2State == LOW) {
        Serial.println("Button 2 pressed");
    }
    if (button3State == LOW) {
        Serial.println("Button 3 pressed");
    }

    // Liten fördröjning för att minska brus
    delay(50);
}