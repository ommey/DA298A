#include <Arduino.h>
#include <painlessMesh.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello, world! from bridge");
  delay(1000);

}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}