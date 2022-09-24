int _density = 5; // 1-8

uint8_t getBrightness(uint32_t ms, uint8_t salt) {
  uint8_t speed = map(setting, 0, NUM_SETTINGS - 1, 3, 7);
  uint16_t ticks = ms >> (8 - speed);
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8(slowcycle16);
  slowcycle16 = (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);

  return ((slowcycle8 & 0x0E) / 2) < _density ? attackDecayWave8(fastcycle8)
                                              : 0;
}

void twinkle() {
  uint16_t PRNG16 = 11337;
  uint32_t clock32 = millis();

  for (uint8_t i = 0; i < STRAND_LENGTH; i++) {
    // Use pseudo random number generator to get values for the clock speed
    // adjustment and clock offset of this pixel
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16 = PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from
    // 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =
        ((((PRNG16 & 0xFF) >> 4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 =
        (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    uint8_t brightness = getBrightness(myclock30, myunique8);

    int paletteIndex = map(i, 0, STRAND_LENGTH, 0, 255);
    CRGB color = ColorFromPalette(currentPalette, paletteIndex);
    leds[i] = color.nscale8(brightness);
  }
}

// This function is like 'triwave8', which produces a
// symmetrical up-and-down triangle sawtooth waveform, except that this
// function produces a triangle wave with a faster attack and a slower decay:
//
//     / \ 
//    /     \ 
//   /         \ 
//  /             \ 
//

uint8_t attackDecayWave8(uint8_t i) {
  if (i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i / 2));
  }
}
