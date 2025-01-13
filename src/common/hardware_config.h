#ifndef HARDWARE_CONFIG_H_
#define HARDWARE_CONFIG_H_

#include <TFT_eSPI.h>
#include <Adafruit_NeoPixel.h>

#define BUTTON_1 GPIO_NUM_22
#define BUTTON_2 GPIO_NUM_32
#define BUTTON_3 GPIO_NUM_33
#define LED_RGB GPIO_NUM_23

extern TFT_eSPI tft;
extern Adafruit_NeoPixel RGB_LED;

void hardwareInit();
void printToDisplay(String message);
void printToDisplay(const String& message, int x, int y);
void printDirection(int fromX, int fromY, int toX, int toY);
void clearDisplay();
void setLEDColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
void setLEDOff();

#endif // HARDWARE_CONFIG_H