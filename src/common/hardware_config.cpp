#include "hardware_config.h"

TFT_eSPI tft = TFT_eSPI();  // Create a TFT_eSPI object
Adafruit_NeoPixel RGB_LED = Adafruit_NeoPixel(1, LED_RGB, NEO_GRB + NEO_KHZ800);

void hardwareInit() 
{
    // Ställ in knapparna som ingångar med pullup
    pinMode(BUTTON_1, INPUT);
    pinMode(BUTTON_2, INPUT);
    pinMode(BUTTON_3, INPUT);

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

void printToDisplay(String message) 
{
    tft.fillScreen(TFT_BLUE);
    tft.setCursor(10, 10);
    tft.print(message);
}

void printToDisplay(const String& message, int x, int y) 
{
    tft.fillScreen(TFT_BLUE);
    tft.setCursor(x, y);
    tft.print(message);
}

void printDirection(int fromX, int fromY, int toX, int toY) 
{
    // sydöst
    if (fromX < toX && fromY < toY)         { tft.drawTriangle(280, 200, 170, 80, 240, 40,  TFT_WHITE); }
    // sydväst
    else if (fromX < toX && fromY > toY)    { tft.drawTriangle(170, 200, 200, 40, 260, 80,  TFT_WHITE); }
    // nordöst
    else if (fromX > toX && fromY < toY)    { tft.drawTriangle(280, 40, 240, 200, 170, 170, TFT_WHITE); }
        // nordväst
    else if (fromX > toX && fromY > toY)    { tft.drawTriangle(170, 40, 240, 200, 280, 170, TFT_WHITE); }
    // öster
    else if (fromX == toX && fromY < toY)   { tft.drawTriangle(280, 120, 120, 80, 120, 160, TFT_WHITE); }
    // väster
    else if (fromX == toX && fromY > toY)   { tft.drawTriangle(120, 120, 280, 160, 280, 80, TFT_WHITE); }
    // söder
    else if (fromX < toX && fromY == toY)   { tft.drawTriangle(240, 200, 200, 40, 280, 40,  TFT_WHITE); }
    // norr
    else if (fromX > toX && fromY == toY)   { tft.drawTriangle(240, 40, 200, 200, 280, 200, TFT_WHITE); } 
}

void clearDisplay() { tft.fillScreen(TFT_BLUE); }

void setLEDColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) 
{
    RGB_LED.setPixelColor(0, RGB_LED.Color(g, r, b, w));
    RGB_LED.show();
}

void setLEDOff() 
{
    RGB_LED.setPixelColor(0, RGB_LED.Color(0, 0, 0));
    RGB_LED.show();
}
