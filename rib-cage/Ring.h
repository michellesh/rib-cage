#define MAX_RING_INDEX 24
#define MAX_RADIUS 8

int RING_INNER[] = {76, 96, 85, 99, 77, 91, 88, 82, 94, 78, 97, 86,
                    89, 79, 83, 92, 95, 80, 98, 84, 87, 90, 93, 81};
int RING_MIDDLE[] = {26, 36, 33, 30, 49, 27, 46, 43, 40, 37, 34, 31,
                     28, 47, 44, 41, 38, 35, 32, 29, 48, 45, 42, 39};
int RING_OUTER[] = {51, 66, 59, 73, 63, 70, 52, 56, 60, 67, 74, 53,
                    64, 71, 57, 61, 68, 54, 65, 72, 58, 62, 69, 55};

int RADIUS_NUCLEUS[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
int RADIUS_INNER[] = {4, 4, 5, 5, 5, 6, 6, 6, 5, 5, 5, 4,
                      4, 4, 5, 5, 5, 6, 6, 6, 5, 5, 5, 4};
int RADIUS_MIDDLE[] = {5, 5, 6, 6, 6, 7, 7, 7, 6, 6, 6, 5,
                       5, 5, 6, 6, 6, 7, 7, 7, 6, 6, 6, 5};
int RADIUS_OUTER[] = {6, 6, 7, 7, 7, 8, 8, 8, 7, 7, 7, 6,
                      6, 6, 7, 7, 7, 8, 8, 8, 7, 7, 7, 6};

class Ring {
private:
  int _index[MAX_RING_INDEX];
  float _speed;
  float _target;
  uint8_t _ringType;
  int _width = 1;
  CRGB _color = CRGB::Yellow;

public:
  static const uint8_t INNER = 0;
  static const uint8_t MIDDLE = 1;
  static const uint8_t OUTER = 2;

  Ring(uint8_t ringType) {
    _ringType = ringType;
    size_t size = sizeof(RING_INNER[0]) * MAX_RING_INDEX;
    switch (ringType) {
    case INNER:
      _target = 0;
      _speed = 0.4;
      _color = CRGB::Yellow;
      memcpy(_index, RING_INNER, size);
      break;
    case MIDDLE:
      _target = 8;
      _speed = -0.3;
      _color = CRGB::Blue;
      memcpy(_index, RING_MIDDLE, size);
      break;
    case OUTER:
      _target = 16;
      _speed = 0.2;
      _color = CRGB::Green;
      memcpy(_index, RING_OUTER, size);
      break;
    default:
      break;
    }
  }

  CRGB getColor(int brightness = 255) {
    return CRGB(_color).nscale8(brightness);
  }

  float getTarget() { return _target; }

  int getWidth() { return _width; }

  int getRadius(int index) {
    switch (_ringType) {
    case INNER:
      return RADIUS_INNER[index];
    case MIDDLE:
      return RADIUS_MIDDLE[index];
    case OUTER:
      return RADIUS_OUTER[index];
    default:
      return 0;
    }
  }

  void setSpeed(float speed) { _speed = speed; }

  void incTarget() {
    _target += _speed;
    if (_target >= MAX_RING_INDEX) {
      _target = 0;
    } else if (_target < 0) {
      _target = MAX_RING_INDEX - abs(_speed);
    }
  }

  void setLED(int index, CRGB color) { leds[_index[index]] = color; }
};
