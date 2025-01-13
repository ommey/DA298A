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
    tft.setRotation(1);
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

void printDirection(int fromX, int fromY, int toX, int toY) {
    if (fromX < toX && fromY < toY) {
        // sydöst
        tft.drawTriangle(150, 150, 170, 170, 130, 170, TFT_WHITE); 
    } else if (fromX < toX && fromY > toY) {
        // sydväst
        tft.drawTriangle(150, 150, 170, 130, 170, 170, TFT_WHITE); 
    } else if (fromX > toX && fromY < toY) {
        // nordöst
        tft.drawTriangle(150, 150, 130, 170, 130, 130, TFT_WHITE); 
    } else if (fromX > toX && fromY > toY) {
        // nordväst
        tft.drawTriangle(150, 150, 130, 130, 170, 130, TFT_WHITE); 
    } else if (fromX == toX && fromY < toY) {
        // öster
        tft.drawTriangle(150, 150, 130, 170, 170, 170, TFT_WHITE); 
    } else if (fromX == toX && fromY > toY) {
        // väster
        tft.drawTriangle(150, 150, 130, 130, 170, 130, TFT_WHITE); 
    } else if (fromX < toX && fromY == toY) {
        // söder
        tft.drawTriangle(150, 150, 170, 170, 130, 170, TFT_WHITE); 
    } else if (fromX > toX && fromY == toY) {
        // norr
        tft.drawTriangle(150, 150, 130, 130, 170, 130, TFT_WHITE); 
    } 
}


void clearDisplay() {
    tft.fillScreen(TFT_BLUE);
}

void setLEDColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    // Not implemented yet
    RGB_LED.setPixelColor(0, RGB_LED.Color(g, r, b, w));
    RGB_LED.show();
}

void setLEDOff() {
    RGB_LED.setPixelColor(0, RGB_LED.Color(0, 0, 0));
    RGB_LED.show();
}
