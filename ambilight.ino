// Ambi: AmbientNeoPixel Arduino Sketch
// Created By: Cory Malantonio
// https://gist.github.com/paganini24/42fdb162a32904a4e8b8d2cba6d86368

#include <Adafruit_NeoPixel.h>
#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(50, PIN, NEO_RGB + NEO_KHZ800);

int r[11]; // array for each color value
int g[11];
int b[11];

void setup() {
  Serial.begin(9600);
  strip.begin(); // prep the neopixel
  strip.show();
}

void loop() {

  if (Serial.available()>=31) {
    if (Serial.read() == 0xff){
      for (int x = 1; x < 11; x++){
        r[x] = Serial.read(); // read in the values from processing
        g[x] = Serial.read(); // in the same order we sent them
        b[x] = Serial.read();

        r[x] = constrain(r[x], 0, 255); // just incase any values slip away
        g[x] = constrain(g[x], 0, 255);
        b[x] = constrain(b[x], 0, 255);

      }
    }
  }


int Xval = 1; // count to 30
int Yval = 2; // while loading rgb values
int Zval = 3; // into 3 led's at a time

for (int z = 1; z < 11; z++){

strip.setPixelColor(Xval, r[z], g[z], b[z]);
strip.setPixelColor(Yval, r[z], g[z], b[z]);
strip.setPixelColor(Zval, r[z], g[z], b[z]);

Xval = Xval + 3;
Yval = Yval + 3;
Zval = Zval + 3;
}

  strip.show(); //output to the neopixel
  delay(20); //for safety
}
