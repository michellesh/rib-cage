void atom() {
  if (setting == 0) {
    atomSolid();
  } else {
    // Nucleus
    for (int i = 1; i < 25; i++) {
      leds[i] = CRGB::Red;
    }

    innerRing.setSpeed(0.3 * 1.05 * setting);
    middleRing.setSpeed(-0.2 * 1.05 * setting);
    outerRing.setSpeed(0.1 * 1.05 * setting);
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
  setRing(innerRing);
  setRing(middleRing);
  setRing(outerRing);
}

void setRing(Ring &ring) {
  for (int i = 0; i < 24; i++) {
    int dist = abs(ring.getTarget() - i);
    dist = minf(dist, abs(ring.getTarget() - 24 - i));
    dist = minf(dist, abs(ring.getTarget() + 24 - i));
    int brightness =
        dist > ring.getWidth() ? 0 : map(dist, 0, ring.getWidth(), 255, 0);

    ring.setLED(i, ring.getColor(brightness));
  }

  ring.incTarget();
}
