/*
 * LED fairy lights wrapped arranged in the shape of an atom
 *
 * REQUIRED LIBRARIES
 * FastLED            Arduino libraries manager
 * ArduinoFFT         Arduino libraries manager
 * EEPROM             Built in
 *
 * WIRING
 * LED data     D2 via 470R resistor
 * GND          GND
 * Vin          5V
 *
 * INMP441 Microphone
 * VDD          3V3
 * GND          GND
 * L/R          GND
 * WS           D15
 * SCK          D14
 * SD           D32
 *
 * Button
 * +            3V3
 * -            D25 and GND via 470 resistor
 *
 * Potentiometers
 * Knob 1 data  D13
 * Knob 2 data  D33
 * Knob 3 data  D27
 *
 * REFERENCES
 * Sound reactive code/algorithm:
 * - Scott Marley https://github.com/s-marley/ESP32-INMP441-Matrix-VU
 * - Andrew Tuline et al https://github.com/atuline/WLED
 */

#include "audio_reactive.h"
#include <EEPROM.h>
#include <FastLED.h>

#include "Timer.h"
#include "colors.h"

#define EEPROM_SIZE 5
#define LED_PIN 2
#define M_WIDTH 8
#define M_HEIGHT 18
#define NUM_LEDS (M_WIDTH * M_HEIGHT)

#define EEPROM_BRIGHTNESS 0
#define EEPROM_GAIN 1
#define EEPROM_SQUELCH 2
#define EEPROM_PATTERN 3
#define EEPROM_DISPLAY_TIME 4

#define LEFT_KNOB_PIN 13
#define MIDDLE_KNOB_PIN 33
#define RIGHT_KNOB_PIN 27
#define BUTTON_PIN 25

#define DEFAULT_BRIGHTNESS 100
#define DEFAULT_GAIN 30
#define DEFAULT_SQUELCH 9

#define PATTERN_SOUND 0
#define PATTERN_TWINKLE 1
#define PATTERN_PRIDE 2
#define PATTERN_HEARTBEAT 3
#define PATTERN_SOLID 4
#define PATTERN_ATOM 5
#define NUM_PATTERNS 6
#define NUM_SETTINGS 5
#define NUM_PALETTES 6

#define STRAND_LENGTH 100

uint8_t numBands;
uint8_t barWidth;
uint8_t pattern;
uint8_t brightness;
uint16_t displayTime;
uint8_t setting = 0;
uint8_t paletteIndex = 0;
int buttonHold = 0;
int calibrateMode = 0;
Timer buttonHoldTimer = {2000};

CRGB knobColors[] = {CRGB::Maroon, CRGB::Orchid, CRGB::Turquoise, CRGB::White,
                     CRGB::FairyLight};
CRGB knobColor = knobColors[0];

CRGB leds[NUM_LEDS];

uint8_t peak[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t prevFFTValue[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t barHeights[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int MAX_BRIGHTNESS = 255;

#include "Ring.h"
Ring innerRing(Ring::INNER);
Ring middleRing(Ring::MIDDLE);
Ring outerRing(Ring::OUTER);

void setup() {
  Serial.begin(57600);
  delay(500);

  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS); // fairy lights

  setupAudio();

  if (M_WIDTH == 8)
    numBands = 8;
  else
    numBands = 16;
  barWidth = M_WIDTH / numBands;

  EEPROM.begin(EEPROM_SIZE);

  // It should not normally be possible to set the gain to 255
  // If this has happened, the EEPROM has probably never been written to
  // (new board?) so reset the values to something sane.
  if (EEPROM.read(EEPROM_GAIN) == 255) {
    EEPROM.write(EEPROM_BRIGHTNESS, 50);
    EEPROM.write(EEPROM_GAIN, 0);
    EEPROM.write(EEPROM_SQUELCH, 0);
    EEPROM.write(EEPROM_PATTERN, 0);
    EEPROM.write(EEPROM_DISPLAY_TIME, 10);
    EEPROM.commit();
  }

  brightness = DEFAULT_BRIGHTNESS;
  gain = DEFAULT_GAIN;
  squelch = DEFAULT_SQUELCH;
  pattern = PATTERN_ATOM;

  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  if (paletteIndex == 0) {
    cycleColorPalette();
  }

  if (pattern == PATTERN_HEARTBEAT) {
    fadeToBlackBy(leds, STRAND_LENGTH, 10);
  } else {
    FastLED.clear();
  }

  // Read button and potentiometers
  EVERY_N_MILLISECONDS(10) {
    readInput();
  }

  uint8_t divisor = 1; // If 8 bands, we need to divide things by 2
  if (numBands == 8)
    divisor = 2; // and average each pair of bands together

  for (int i = 0; i < 16; i += divisor) {
    uint8_t fftValue;

    if (numBands == 8)
      fftValue = (fftResult[i] + fftResult[i + 1]) /
                 2; // Average every two bands if numBands = 8
    else
      fftValue = fftResult[i];

    fftValue = ((prevFFTValue[i / divisor] * 3) + fftValue) /
               4; // Dirty rolling average between frames to reduce flicker
    barHeights[i / divisor] = fftValue / (255 / M_HEIGHT); // Scale bar height

    if (barHeights[i / divisor] > peak[i / divisor]) // Move peak up
      peak[i / divisor] = min(M_HEIGHT, (int)barHeights[i / divisor]);

    prevFFTValue[i / divisor] =
        fftValue; // Save prevFFTValue for averaging later
  }

  drawPatterns();

  // Decay peak
  EVERY_N_MILLISECONDS(60) {
    for (uint8_t band = 0; band < numBands; band++)
      if (peak[band] > 0)
        peak[band] -= 1;
  }

  EVERY_N_SECONDS(30) {
    // Save values in EEPROM. Will only be commited if values have changed.
    EEPROM.write(EEPROM_BRIGHTNESS, brightness);
    EEPROM.write(EEPROM_GAIN, gain);
    EEPROM.write(EEPROM_SQUELCH, squelch);
    EEPROM.write(EEPROM_PATTERN, pattern);
    EEPROM.write(EEPROM_DISPLAY_TIME, displayTime);
    EEPROM.commit();
  }

  FastLED.setBrightness(brightness);
  FastLED.show();
  delay(10);
}

void drawPatterns() {
  switch (pattern) {
  case PATTERN_SOUND:
    soundReactive();
    break;
  case PATTERN_TWINKLE:
    twinkle();
    break;
  case PATTERN_PRIDE:
    pride();
    break;
  case PATTERN_HEARTBEAT:
    heartbeat();
    break;
  case PATTERN_ATOM:
    atom();
    break;
  case PATTERN_SOLID:
    for (uint16_t i = 0; i < STRAND_LENGTH; i++) {
      leds[i] = knobColor;
    }
    break;
  default:
    break;
  }
}

void flashLEDs() {
  FastLED.clear();
  FastLED.show();
  delay(200);

  for (int i = 0; i < STRAND_LENGTH; i++) {
    leds[i] = CRGB(255, 0, 0).nscale8(50);
  }
  FastLED.show();
  delay(200);

  FastLED.clear();
  FastLED.show();
  delay(200);

  for (int i = 0; i < STRAND_LENGTH; i++) {
    leds[i] = CRGB(255, 0, 0).nscale8(50);
  }
  FastLED.show();
  delay(200);

  FastLED.clear();
  FastLED.show();
  delay(200);
}
