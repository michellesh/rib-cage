float minf(float x, float y) { return x < y ? x : y; }

int width = 5;

void atom() {
  if (setting == 0) {
    atomSolid();
  } else {
    innerRing.setSpeed(setting);
    middleRing.setSpeed(setting);
    outerRing.setSpeed(setting);
    atomSpin();
  }
}

void atomSolid() {
  // Nucleus
  for (int i = 1; i < 25; i++) {
    leds[i] = CRGB::Red;
  }
  for (int i = 0; i < 24; i++) {
    innerRing.setLED(i, innerRing.getColor(brightness));
    middleRing.setLED(i, middleRing.getColor(brightness));
    outerRing.setLED(i, outerRing.getColor(brightness));
  }
}

void atomSpin() {
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
