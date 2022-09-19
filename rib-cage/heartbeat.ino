int firstBeat = 700;
int secondBeat = 350;
Timer beat = {firstBeat};

void heartbeat() {
  if (beat.complete()) {
    for (uint16_t i = 0; i < STRAND_LENGTH; i++) {
      if ((i % 25 >= 13 && beat.totalCycleTime == firstBeat) ||
          (i % 25 < 13 && beat.totalCycleTime == secondBeat)) {
        leds[i] = knobColor;
      }
    }
    beat.totalCycleTime =
        beat.totalCycleTime == firstBeat ? secondBeat : firstBeat;
    beat.reset();
  }
}
