void soundReactive() {
  switch (setting) {
  default:
  case 0:
    barHeightToBrightness();
    break;
  case 1:
    barHeightSumToStrand();
    break;
  case 2:
    barHeightToMirrorRing();
    break;
  case 3:
    barHeightToSpeed();
    break;
  case 4:
    barHeightToRadius();
    break;
  }
}

void barHeightToBrightness() {
  // Only map to a max brightness of 100 because brightness values >100 are hard
  // to perceive with the eye
  int sumNucleus = mapBarsTo(0, 1, 100);
  int sumInnerRing = mapBarsTo(2, 3, 100);
  int sumMiddleRing = mapBarsTo(4, 5, 100);
  int sumOuterRing = mapBarsTo(6, 7, 100);

  // Nucleus
  for (int i = 1; i < 25; i++) {
    leds[i] = ColorFromPalette(currentPalette, 0).nscale8(sumNucleus);
  }
  // Rings
  for (int i = 0; i < 24; i++) {
    innerRing.setLED(i, innerRing.getColor(sumInnerRing));
    middleRing.setLED(i, middleRing.getColor(sumMiddleRing));
    outerRing.setLED(i, outerRing.getColor(sumOuterRing));
  }
}

void barHeightSumToStrand() {
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

void barHeightToMirrorRing() {
  int sumNucleus = mapBarsTo(0, 1, 100);
  int sumInnerRing = mapBarsTo(2, 3, STRAND_LENGTH / 2);
  int sumMiddleRing = mapBarsTo(4, 5, STRAND_LENGTH / 2);
  int sumOuterRing = mapBarsTo(6, 7, STRAND_LENGTH / 2);

  // Nucleus
  for (int i = 1; i < 25; i++) {
    leds[i] = ColorFromPalette(currentPalette, 0).nscale8(sumNucleus);
  }

  // Rings
  for (int i = 0; i < min(sumInnerRing, 12); i++) {
    innerRing.setLED(i, innerRing.getColor(map(i, 0, sumInnerRing, 0, 255)));
    innerRing.setLED(23 - i, innerRing.getColor());
  }
  for (int i = 0; i < min(sumMiddleRing, 12); i++) {
    middleRing.setLED(i, middleRing.getColor(map(i, 0, sumMiddleRing, 0, 255)));
    middleRing.setLED(23 - i, middleRing.getColor());
  }
  for (int i = 0; i < min(sumOuterRing, 12); i++) {
    outerRing.setLED(i, outerRing.getColor(map(i, 0, sumOuterRing, 0, 255)));
    outerRing.setLED(23 - i, outerRing.getColor());
  }
}

void barHeightToSpeed() {
  // Nucleus
  int sumNucleus = mapBarsTo(0, 1, 255);
  for (int i = 1; i < 25; i++) {
    leds[i] = ColorFromPalette(currentPalette, 0).nscale8(sumNucleus);
  }

  // Rings
  int sumInnerRing = mapBarsTo(2, 3, 4);
  int sumMiddleRing = mapBarsTo(4, 5, 4);
  int sumOuterRing = mapBarsTo(6, 7, 4);

  innerRing.setSpeed(maxf(0.1, sumInnerRing));
  middleRing.setSpeed(maxf(0.1, sumMiddleRing));
  outerRing.setSpeed(maxf(0.1, sumOuterRing));

  atomSpin();
}

void barHeightToRadius() {
  int sum = 0;
  for (int band = 0; band < numBands; band++) {
    uint8_t barHeight = barHeights[band];
    sum += barHeight;
  }

  int radius = map(sum, 0, NUM_LEDS, 0, MAX_RADIUS * 4);
  for (int i = 0; i < 24; i++) {
    // Nucleus
    if (RADIUS_NUCLEUS[i] <= radius) {
      leds[i + 1] = ColorFromPalette(currentPalette, 0);
    }
    // Rings
    if (innerRing.getRadius(i) <= radius) {
      innerRing.setLED(i, innerRing.getColor());
    }
    if (middleRing.getRadius(i) <= radius) {
      middleRing.setLED(i, middleRing.getColor());
    }
    if (outerRing.getRadius(i) <= radius) {
      outerRing.setLED(i, outerRing.getColor());
    }
  }
}

int mapBarsTo(int barIndex1, int barIndex2, int mapToValue) {
  return map(barHeights[barIndex1] + barHeights[barIndex2], 0, M_HEIGHT * 2, 0,
             mapToValue);
}

int mapPeakTo(int barIndex1, int barIndex2, int mapToValue) {
  return map(peak[barIndex1] + peak[barIndex2], 0, M_HEIGHT * 2, 0, mapToValue);
}
