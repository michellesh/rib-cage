#define SECONDS_PER_PALETTE 10

DEFINE_GRADIENT_PALETTE(_redRoseOrchid){0,   128, 0,   0,    // CRGB::Maroon
                                        85,  210, 105, 30,   // CRGB::Chocolate
                                        170, 255, 127, 80,   // CRGB::Coral
                                        255, 218, 112, 214}; // CRGB::Orchid
CRGBPalette16 redRoseOrchid = _redRoseOrchid;

DEFINE_GRADIENT_PALETTE(_tealGreenGold){0,   34,  139, 34, // CRGB::ForestGreen
                                        85,  0,   255, 0,  // CRGB::Lime
                                        170, 255, 215, 0,  // CRGB::Gold
                                        255, 255, 140, 0}; // CRGB::DarkOrange
CRGBPalette16 tealGreenGold = _tealGreenGold;

DEFINE_GRADIENT_PALETTE(_firePalette){0,   255, 0,   0,  // CRGB::Red
                                      200, 255, 69,  0,  // CRGB::OrangeRed
                                      225, 255, 140, 0,  // CRGB::Orange
                                      255, 255, 215, 0}; // CRGB::Gold
CRGBPalette16 firePalette = _firePalette;

DEFINE_GRADIENT_PALETTE(_icePalette){0,   224, 240, 255, // light blue
                                     127, 31,  147, 255, // medium blue
                                     255, 48,  64,  72}; // dark blue
CRGBPalette16 icePalette = _icePalette;

DEFINE_GRADIENT_PALETTE(_fairyPalette){0,   63,  57,  11,   // "QuarterFairy"
                                       127, 127, 114, 22,   // "HalfFairy"
                                       224, 255, 227, 45,   // CRGB::FairyLight
                                       255, 255, 255, 255}; // full white
CRGBPalette16 fairyPalette = _fairyPalette;

// clang-format off
CRGBPalette16* activePalettes[] = {
  &redRoseOrchid,
  &tealGreenGold,
  &firePalette,
  &fairyPalette,
  &icePalette
};
// clang-format on

CRGBPalette16 currentPalette = *(activePalettes[0]);
CRGBPalette16 targetPalette = *(activePalettes[0]);

void setNextColorPalette() {
  const uint8_t numberOfPalettes =
      sizeof(activePalettes) / sizeof(activePalettes[0]);
  static uint8_t whichPalette = -1;
  whichPalette = addmod8(whichPalette, 1, numberOfPalettes);

  targetPalette = *(activePalettes[whichPalette]);
}

void setCurrentColorPalette(uint8_t paletteIndex) {
  currentPalette = *(activePalettes[paletteIndex]);
  targetPalette = *(activePalettes[paletteIndex]);
}

void cycleColorPalette() {
  EVERY_N_SECONDS(SECONDS_PER_PALETTE) { setNextColorPalette(); }

  EVERY_N_MILLISECONDS(10) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, 12);
  }
}
