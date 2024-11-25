#include "hardware_config.h"

TFT_eSPI tft = TFT_eSPI();  // Create a TFT_eSPI object

bool hardwareInit() {
    
    // Ställ in knapparna som ingångar med pullup
    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);
    pinMode(BUTTON_3, INPUT_PULLUP);
}
