/* Example usage:

Timer myTimer = {5000}; // 5 seconds
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
