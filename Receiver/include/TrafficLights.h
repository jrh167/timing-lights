#pragma once
#include "FastLED.h"

/**
 * Display the given colour on the LED grid specified by the offset.
 *
 * @param leds   Array of all CRGB LEDs.
 * @param colour The hex code of the colour to display.
 * @param offset The index of the first LED in the grid.
 */
void displayColour(CRGB *leds, uint32_t colour, uint32_t offset) {
    for (int i = 0; i < 30; ++i) {
        leds[i + offset] = colour;
    }
}

void displayRedLight(CRGB leds[]) {
    displayColour(leds, 0xFF0000, 221);
}

void displayAmberLight(CRGB *leds) {
    displayColour(leds, 0xFF8000, 191);
}

void displayGreenLight(CRGB leds[]) {
    displayColour(leds, 0x00FF00, 161);
}

void clearRedLight(CRGB leds[]) {
    displayColour(leds, 0x000000, 221);
}

void clearAmberLight(CRGB *leds) {
    displayColour(leds, 0x000000, 191);
}

void clearGreenLight(CRGB leds[]) {
    displayColour(leds, 0x000000, 161);
}
