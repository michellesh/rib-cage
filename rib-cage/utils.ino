float mapf(float value, float inMin, float inMax, float outMin, float outMax) {
  float percentage = (value - inMin) / (inMax - inMin);
  return outMin + (outMax - outMin) * percentage;
}

float minf(float a, float b) { return a < b ? a : b; }

float maxf(float a, float b) { return a > b ? a : b; }
