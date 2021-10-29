#pragma once
#include "FastLED.h"

/**
 * Binary definitions for the colouration of 7-seg characters (4 LEDs per seg).
 * 0 = off, 1 = 0xFFFF00 (yellow)
 */
const uint32_t OFF = 0b00000000000000000000000000000000;
const uint32_t A =   0b00000000111111111111111111111111;
const uint32_t B =   0b11111111111111111111111111111111;
const uint32_t C =   0b00001111111100001111111110000001;
const uint32_t D =   0b00001111111111111111111100001111;

/**
 * Draw the given letter on the detail display, in the position given by the offset value.
 *
 * @param leds   Array of all CRGB LEDs.
 * @param letter The binary pattern for the letter, where 0 is off and 1 is yellow.
 * @param offset The index of the first LED in the 7-seg array.
 */
void displayDetail(CRGB leds[], const uint32_t letter, uint32_t offset) {
    for (int i = 0; i < 28; ++i) {
        leds[i + offset] = (letter >> i) & 1 ? 0xFFFF00 : 0x000000;
    }
}

void displayA(CRGB leds[]) {
    displayDetail(leds, A, 28);
}

void displayB(CRGB leds[]) {
    displayDetail(leds, B, 0);
}

void displayC(CRGB leds[]) {
    displayDetail(leds, C, 28);
}

void displayD(CRGB leds[]) {
    displayDetail(leds, D, 0);
}

void clearAC(CRGB leds[]) {
    displayDetail(leds, OFF, 28);
}

void clearBD(CRGB leds[]) {
    displayDetail(leds, OFF, 0);
}
