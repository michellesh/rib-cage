/*
 * Web enabled FFT VU meter for a matrix, ESP32 and INMP441 digital mic.
 * The matrix width MUST be either 8 or a multiple of 16 but the height can
 * be any value. E.g. 8x8, 16x16, 8x10, 32x9 etc.
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
 * INMP441
 * VDD          3V3
 * GND          GND
 * L/R          GND
 * WS           D15
 * SCK          D14
 * SD           D32
 *
 * REFERENCES
 * Main code      Scott Marley            https://www.youtube.com/c/ScottMarley
 * Andrew Tuline et al     https://github.com/atuline/WLED
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

#define BRIGHTNESS_PIN 13
#define GAIN_PIN 33
#define SQUELCH_PIN 27
#define BUTTON_PIN 25
#define COLOR_PIN 26

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

#define STRAND_LENGTH 100

uint8_t numBands;
uint8_t barWidth;
uint8_t pattern;
uint8_t brightness;
uint16_t displayTime;
uint8_t setting = 0;
int buttonState = 0;

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
  cycleColorPalette();

  if (pattern == PATTERN_HEARTBEAT) {
    fadeToBlackBy(leds, STRAND_LENGTH, 10);
  } else {
    FastLED.clear();
  }

  // Read button and potentiometers
  EVERY_N_MILLISECONDS(100) {
    int buttonRead = digitalRead(BUTTON_PIN);
    if (buttonRead == HIGH && buttonState == 0) {
      buttonState = 1;
      pattern = (pattern + 1) % NUM_PATTERNS; // Increment pattern
    } else if (buttonRead == LOW && buttonState == 1) {
      buttonState = 0;
    }

    int value = analogRead(SQUELCH_PIN);
    if (pattern == PATTERN_SOUND) {
      squelch = map(value, 4095, 0, 0, 30);
    } else {
      int numColors = sizeof(knobColors) / sizeof(knobColors[0]);
      int colorIndex = map(value, 4095, 0, 0, numColors - 1);
      knobColor = knobColors[colorIndex];
    }

    value = analogRead(GAIN_PIN);
    if (pattern == PATTERN_SOUND) {
      gain = map(value, 4095, 0, 0, 30);
    } else {
      setting = map(value, 4095, 0, 0, NUM_SETTINGS);
    }

    brightness = map(analogRead(BRIGHTNESS_PIN), 4095, 0, 0, 255);
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
  default: // PATTERN_SOLID
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = knobColor;
    }
    break;
  }
}

//////////// Patterns ////////////

void rainbowBars(uint8_t band, uint8_t barHeight) {
  // from the beginning of each segment
  // int iStart = M_WIDTH * band;
  // for (int i = iStart; i < iStart + barHeight; i++) {
  //  leds[i] = CHSV(band * (255 / numBands), 255, 255);
  //}

  // from the middle of each segment
  int i = M_WIDTH * band;
  int half = M_WIDTH / 2;
  for (int x = 0; x < barHeight / 2; x++) {
    leds[i + half + x] = CHSV(band * (255 / numBands), 255, 255);
    leds[i + half - 1 - x] = CHSV(band * (255 / numBands), 255, 255);
  }
}

void barAverage() {
  int sum = 0;
  for (int band = 0; band < numBands; band++) {
    uint8_t barHeight = barHeights[band];
    sum += barHeight;
  }
  int averageHeight = sum / numBands;
  int height = map(averageHeight, 0, M_HEIGHT, 0, NUM_LEDS / 4);

  int middle = NUM_LEDS / 2;
  for (int x = 0; x < height; x++) {
    leds[x] = CHSV(x * (255 / numBands), 255, 255);
    leds[middle + x] = CHSV(x * (255 / numBands), 255, 255);
    leds[middle - 1 - x] = CHSV(x * (255 / numBands), 255, 255);
    leds[NUM_LEDS - 1 - x] = CHSV(x * (255 / numBands), 255, 255);
  }
}

void barSum() {
  int sum = 0;
  for (int band = 0; band < numBands; band++) {
    uint8_t barHeight = barHeights[band];
    sum += barHeight;
  }
  int height = map(sum, 0, NUM_LEDS, 0, NUM_LEDS / 2);

  int middle = NUM_LEDS / 2;
  for (int x = 0; x < height; x++) {
    int hue = map(x, 0, NUM_LEDS / 4, 0, 255);
    leds[x] = CHSV(hue, 255, 255);
    leds[middle + x] = CHSV(hue, 255, 255);
    leds[middle - 1 - x] = CHSV(hue, 255, 255);
    leds[NUM_LEDS - 1 - x] = CHSV(hue, 255, 255);
  }
}

void soundReactive() {
  int numStrands = 4;
  int offset = 2;

  int sum = 0;
  for (int band = 0; band < numBands; band++) {
    uint8_t barHeight = barHeights[band];
    sum += barHeight;
  }

  // height gets mapped to full STRAND_LENGTH of 100 instead of 25 (length of
  // each substrand) as a way of *4 amplifying the mic "sensitivity". I also did
  // this manually with `* 2` on my bike
  int height = map(sum, 0, NUM_LEDS, 0, STRAND_LENGTH);
  for (int s = 0; s < numStrands; s++) {
    for (int x = 0; x < height; x++) {
      int index = x + (s * 25) + offset;
      if (index < STRAND_LENGTH) {
        int brightness = map(x, 0, STRAND_LENGTH, 0, 255);
        CRGB color = ColorFromPalette(currentPalette, brightness);
        leds[index] = color.nscale8(255 - brightness);
      }
    }
  }
}
