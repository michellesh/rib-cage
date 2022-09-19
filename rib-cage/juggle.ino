// Eight colored dots, weaving in and out of sync with each other

void juggle() {
  fadeToBlackBy(leds, STRAND_LENGTH, 20);
  uint8_t dothue = 0;
  for (int j = 0; j < 8; j++) {
    leds[beatsin16(j + 7, 0, STRAND_LENGTH - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
