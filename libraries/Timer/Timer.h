/* Example usage:

Timer myTimer = {cycleTime};
if (myTimer.complete()) {
   ...
   myTimer.reset();
}
*/

struct Timer {
  unsigned long totalCycleTime;
  unsigned long lastCycleTime;
  void reset() {
    lastCycleTime = millis();
  };
  bool complete() {
    return (millis() - lastCycleTime) > totalCycleTime;
  };
};
