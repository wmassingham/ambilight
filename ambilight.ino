// Ambi: AmbientNeoPixel Arduino Sketch
// Created By: Cory Malantonio
// https://gist.github.com/paganini24/42fdb162a32904a4e8b8d2cba6d86368
// Modified by Webster Massingham, https://github.com/wmassingham/ambilight/blob/master/ambilight.ino

#include <Adafruit_NeoPixel.h>
#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, PIN, NEO_GRBW + NEO_KHZ800);

int r[11]; // array for each color value
int g[11];
int b[11];

unsigned long lastTime = 0; // time of last valid input, in ms since boot

void setup() {
  Serial.begin(15200);
  strip.begin(); // prep the neopixel
  strip.show();
}

void loop() {

  unsigned long currentTime = millis();

  if (Serial.available() >= 31) {

    if (Serial.read() == 0xff) {
      lastTime = millis();
      for (int x = 1; x < 11; x++) {
        r[x] = Serial.read(); // read in the values from processing
        g[x] = Serial.read(); // in the same order we sent them
        b[x] = Serial.read();

        r[x] = constrain(r[x], 0, 255); // just incase any values slip away
        g[x] = constrain(g[x], 0, 255);
        b[x] = constrain(b[x], 0, 255);
      }
    }

    int Xval = 1; // count to 30
    int Yval = 2; // while loading rgb values
    int Zval = 3; // into 3 led's at a time

    for (int z = 1; z < 11; z++) {

      strip.setPixelColor(Xval, r[z], g[z], b[z]);
      strip.setPixelColor(Yval, r[z], g[z], b[z]);
      strip.setPixelColor(Zval, r[z], g[z], b[z]);

      Xval = Xval + 3;
      Yval = Yval + 3;
      Zval = Zval + 3;

    }

    strip.show(); //output to the neopixel
    delay(20); //for safety

  } else if (currentTime - lastTime > 21000) {

    // no data for a while, must be disconnected
    // turn off the LEDs and wait for input
    for (int i = 0; i < 31; i++) { strip.setPixelColor(i, 0, 0, 0); }
    strip.show();
    delay(1000);

  }

}
