float minf(float x, float y) { return x < y ? x : y; }

int width = 5;

void atom() {
  // Nucleus
  for (int i = 1; i < 25; i++) {
    leds[i] = CRGB::Red;
  }

  setRing(innerRing);
  setRing(middleRing);
  setRing(outerRing);
}

void setRing(Ring &ring) {
  for (int i = 0; i < 24; i++) {
    int dist = abs(ring.getTarget() - i);
    dist = minf(dist, abs(ring.getTarget() - 24 - i));
    dist = minf(dist, abs(ring.getTarget() + 24 - i));
    int brightness = dist > width ? 0 : map(dist, 0, width, 255, 0);

    ring.setLED(i, ring.getColor(brightness));
  }

  ring.incTarget();
}
