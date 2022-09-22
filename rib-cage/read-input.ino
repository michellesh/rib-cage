void readInput() {
  int buttonRead = digitalRead(BUTTON_PIN); // HIGH when button is held
  if (buttonRead == HIGH && buttonHold == 0) {
    buttonHold = 1;
    buttonHoldTimer.reset();
  } else if (buttonRead == LOW && buttonHold == 1) {
    if (!calibrateMode) {
      pattern = (pattern + 1) % NUM_PATTERNS; // Increment pattern
    }
    buttonHold = 0;
    calibrateMode = 0;
  }

  // User enters "calibrate mode" when button is held down for >2
  // seconds and the active pattern is the sound reactive pattern
  // Flash all LEDs red twice when user enters calibrate mode
  if (calibrateMode == 0 && pattern == PATTERN_SOUND && buttonHold &&
      buttonHoldTimer.complete()) {
    flashLEDs();
    calibrateMode = 1;
  }

  // Only update brightness if there was a significant change in reading (>10)
  // Seems like any sensitivity below 10 causes flickering
  int brightnessSensitivity = 10;
  int _brightness = map(analogRead(LEFT_KNOB_PIN), 4095, 0, 0, 255);
  if (abs(_brightness - brightness) > brightnessSensitivity) {
    brightness = _brightness;
  }

  int middleKnobValue = analogRead(MIDDLE_KNOB_PIN);
  int rightKnobValue = analogRead(RIGHT_KNOB_PIN);

  // CALIBRATE MODE: adjust microphone sensitivity (gain) and squelch
  if (calibrateMode) {
    gain = map(middleKnobValue, 4095, 0, 0, 30);
    squelch = map(rightKnobValue, 4095, 0, 0, 30);

  } else { // REGULAR SETTINGS MODE
    setting = map(middleKnobValue, 4095, 0, 0, NUM_SETTINGS - 1);

    uint8_t numColors = sizeof(knobColors) / sizeof(knobColors[0]);
    uint8_t knobColorIndex = map(rightKnobValue, 4095, 0, 0, numColors - 1);
    knobColor = knobColors[knobColorIndex];

    paletteIndex = map(rightKnobValue, 4095, 0, 0, NUM_PALETTES - 1);
    if (paletteIndex > 0) {
      setCurrentColorPalette(paletteIndex - 1);
    }
  }
}

void printInputValues() {
  EVERY_N_MILLISECONDS(500) {
    Serial.print("pattern: ");
    Serial.println(pattern);
    if (calibrateMode) {
      Serial.print("--CALIBRATE MODE--");
      Serial.print("gain: ");
      Serial.println(gain);
      Serial.print("squelch: ");
      Serial.println(squelch);
    } else {
      int rightKnobValue = analogRead(RIGHT_KNOB_PIN);
      uint8_t numColors = sizeof(knobColors) / sizeof(knobColors[0]);
      uint8_t knobColorIndex = map(rightKnobValue, 4095, 0, 0, numColors - 1);
      Serial.print("knobColorIndex: ");
      Serial.println(knobColorIndex);
      Serial.print("setting: ");
      Serial.println(setting);
    }
    Serial.println();
  }
}
