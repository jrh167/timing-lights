#pragma once
#include "FastLED.h"

/**
 * Binary definitions for the colouration of 7-seg characters (5 LEDs per seg).
 * 0 = off, 1 = 0xFFFF00 (yellow)
 */
const uint64_t NUMBER_OFF = 0b0000000000000000000000000000000000000000000000000000000000000000;
const uint64_t ZERO =       0b0000000000000000000000000000000000111111111111111111111111111111;
const uint64_t ONE =        0b0000000000000000000000000000000000000000000011111111110000000000;
const uint64_t TWO =        0b0000000000000000000000000000011111000001111111111000001111111111;
const uint64_t THREE =      0b0000000000000000000000000000011111000001111111111111111111100000;
const uint64_t FOUR =       0b0000000000000000000000000000011111111110000011111111110000000000;
const uint64_t FIVE =       0b0000000000000000000000000000011111111111111100000111111111100000;
const uint64_t SIX =        0b0000000000000000000000000000011111111111111100000111111111111111;
const uint64_t SEVEN =      0b0000000000000000000000000000000000000001111111111111110000000000;
const uint64_t EIGHT =      0b0000000000000000000000000000011111111111111111111111111111111111;
const uint64_t NINE =       0b0000000000000000000000000000011111111111111111111111111111100000;

const uint64_t NUMBERS[11] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, NUMBER_OFF};

const uint32_t HUNDREDS_OFFSET = 56;
const uint32_t TENS_OFFSET = 91;
const uint32_t ONES_OFFSET = 126;

/**
 * Turns off all LEDs in the numerical display.
 *
 * @param leds Array of all CRGB LEDs.
 */
void clearNumber(CRGB leds[]) {
    for (int i = HUNDREDS_OFFSET; i < 161; ++i) {
        leds[i] = 0x000000;
    }
}

/**
 * Turns off all LEDs of a digit in the numerical display, specified by the offset.
 *
 * @param leds   Array of all CRGB LEDs.
 * @param offset The index of the first LED in the 7-seg array.
 */
void clearDigit(CRGB leds[], uint32_t offset) {
    for (int i = 0; i < 35; ++i) {
        leds[i + offset] = 0x000000;
    }
}

/**
 * Draw the given digit on the numerical display, in the position given by the offset value.
 *
 * @param leds   Array of all CRGB LEDs.
 * @param offset The index of the first LED in the 7-seg array.
 * @param digit  The binary pattern for the digit, where 0 is off and 1 is yellow.
 */
void displayDigit(CRGB leds[], uint32_t offset, uint32_t digit) {
    const auto segs = NUMBERS[digit];
    for (int i = 0; i < 35; ++i) {
        leds[i + offset] = (segs >> i) & 1 ? 0xFFFF00 : 0x000000;
    }
}

/**
 * Draw the given number on the numerical display.
 *
 * @param leds   Array of all CRGB LEDs.
 * @param number The at-most 3-digit number to display.
 */
void displayNumber(CRGB leds[], uint32_t number) {
    const uint32_t hundreds = number / 100;
    const uint32_t tens = (number - (hundreds * 100)) / 10;
    const uint32_t ones = number - (hundreds * 100) - (tens * 10);

    if (hundreds == 0) {
        clearDigit(leds, HUNDREDS_OFFSET);
    } else {
        displayDigit(leds, HUNDREDS_OFFSET, hundreds);
    }

    if (tens == 0 && hundreds == 0) {
        clearDigit(leds, TENS_OFFSET);
    } else {
        displayDigit(leds, TENS_OFFSET, tens);
    }

    displayDigit(leds, ONES_OFFSET, ones);
}
