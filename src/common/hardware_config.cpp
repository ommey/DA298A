#include "hardware_config.h"

TFT_eSPI tft = TFT_eSPI();  // Create a TFT_eSPI object
Adafruit_NeoPixel RGB_LED = Adafruit_NeoPixel(1, LED_RGB, NEO_GRB + NEO_KHZ800);

void hardwareInit() {
    
    // Ställ in knapparna som ingångar med pullup
    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);
    pinMode(BUTTON_3, INPUT_PULLUP);

    // Ställ in RGB-LED som utgång
    RGB_LED.begin();
    RGB_LED.show();

    // Init display
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    //tft.print("Hello, World!");
}

void printToDisplay(String message) {
    tft.fillScreen(TFT_BLUE);
    tft.setCursor(10, 10);
    tft.print(message);
}

void printToDisplay(const String& message, int x, int y) {
    tft.fillScreen(TFT_BLUE);
    tft.setCursor(x, y);
    tft.print(message);
}

void printDirection(int x_from, int y_from, int x_to, int y_to) {
    tft.fillScreen(TFT_BLUE);
    tft.setCursor(10, 20);
    tft.print("Move to ");
    tft.print(x_to + ',' + y_to + " from " + x_from + ',' + y_from);
}

void clearDisplay() {
    tft.fillScreen(TFT_BLUE);
    tft.setCursor(10, 10);
}

void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
    // Not implemented yet
    RGB_LED.setPixelColor(0, RGB_LED.Color(r, g, b));
    RGB_LED.show();
}

void setLEDOff() {
    RGB_LED.setPixelColor(0, RGB_LED.Color(0, 0, 0));
    RGB_LED.show();
}
