float sinwave(float minValue, float maxValue, unsigned long waveLength = 50,
              unsigned long waveLengthOffset = 0) {
  return mapf(sin((float)(ticks + waveLengthOffset) * PI / waveLength), -1, 1,
              minValue, maxValue);
}

void heartbeat() {
  ticks++;

  unsigned long waveLength = map(setting, 0, NUM_SETTINGS - 1, 45, 25);
  unsigned long waveLengthOffset = (float)waveLength * 0.7;
  static uint8_t firstBeatRadius = -1;
  static uint8_t secondBeatRadius = -1;
  firstBeatRadius = maxf(0, sinwave(-16, 8, waveLength));
  secondBeatRadius = maxf(0, sinwave(-16, 6, waveLength, waveLengthOffset));

  for (uint8_t i = 0; i < 24; i++) {
    if (RADIUS_NUCLEUS[i] <= firstBeatRadius ||
        RADIUS_NUCLEUS[i] <= secondBeatRadius) {
      leds[i] = knobColor;
    }
    if (innerRing.getRadius(i) <= firstBeatRadius ||
        innerRing.getRadius(i) <= secondBeatRadius) {
      innerRing.setLED(i, knobColor);
    }
    if (middleRing.getRadius(i) <= firstBeatRadius ||
        middleRing.getRadius(i) <= secondBeatRadius) {
      middleRing.setLED(i, knobColor);
    }
    if (outerRing.getRadius(i) <= firstBeatRadius ||
        outerRing.getRadius(i) <= secondBeatRadius) {
      outerRing.setLED(i, knobColor);
    }
  }
}
