#include <Arduino.h>
#include <../lib/ByteBuf/include/ByteBuf.h>
#include "FastLED.h"
#include <DetailLEDs.h>
#include <TrafficLights.h>
#include <NumericLEDs.h>

#define DEBUG_LOGGING
//#define VERBOSE_DEBUG_LOGGING
#define QUIET_PIN 7
#define MATCHPLAY_PIN 8
#define BRIGHTNESS_PIN 9
#define LED_PIN 10
#define BUZZER_PIN 11
#define LOUD_PIN 12
#define CHIPSET WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS 251
#define HIGH_BRIGHTNESS 192
#define LOW_BRIGHTNESS 64
#define MUTE 0
#define QUIET 64
#define LOUD 254

const byte HEADER[4] = {(byte) 0xA4, 0x11, (byte) 0xE4, (byte) 0xD8};
const char *HEX_CHARS = "0123456789ABCDEF";

// Packet format:
// ____ ___L XYCD DccT
// _ = Unused
// L = Last end (matchplay)
// X = Emergency Stop
// Y = Matchplay
// C = Countdown
// D = Detail
// c = Colour
// T = TimeEnabled

struct State {
   bool countdownContinues;  // If the countdown currently ending will be followed by another
   bool lastEnd;             // If the current end is the last (for matchplay)
   bool countdown;           // If the unit should count down the seconds
   byte detail;              // The detail being displayed (none, A/B, or C/D)
   byte colour;              // The colour on the traffic light (red, amber, or green)
   bool timeEnabled;         // If the time should be displayed on the panel
   int time;                 // The time to display
   int startNumBeeps;        // Number of beeps to sound at the start of the countdown
   int endNumBeeps;          // Number of beeps to sound at the end of the countdown
};

const byte DETAIL_OFF = 0;
const byte DETAIL_AB = 1;
const byte DETAIL_CD = 2;

const byte RED = 0;
const byte AMBER = 1;
const byte GREEN = 2;

const int BUZZER_DURATION = 500;   // How long the buzzer should sound on/off for

ByteBuf buffer(320);
int expectedSize = -1;
State state;
CRGB leds[NUM_LEDS];
unsigned long startTime;
bool buzzerIsActive;
unsigned long buzzerStart;
unsigned long buzzerEnd;
bool ledsDirty;
int oldBrightness;
byte matchplayMode;

void printBuffer(const String &prefix, ByteBuf &buf);

unsigned int makeChecksum(ByteBuf &buf);

void handlePacket(ByteBuf &buf);

void configureBrightness();

void enterBlankState();

bool isBitSet(byte index, int b) {
    return ((1 << index) & b) != 0;
}

/**
 * Initialize serial communications, LEDs, and Arduino pins.
 */
void setup() {
    Serial.begin(9600);

    // Initialize pins
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(MATCHPLAY_PIN, INPUT_PULLUP);
    pinMode(BRIGHTNESS_PIN, INPUT_PULLUP);
    pinMode(LOUD_PIN , INPUT_PULLUP);
    pinMode(QUIET_PIN , INPUT_PULLUP);

    TCCR2B = TCCR2B & 0b11111000 | 0x01;

    // Initialize LEDs to off
    CFastLED::addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    oldBrightness = -1;
    configureBrightness();
    for (auto & led : leds) {
        led = 0x000000;
    }

    enterBlankState();
}

/**
 * Schedule the buzzer the number of times specified in 800ms pulses.
 *
 * @param times The number of times the buzzer should sound.
 */
void beep(int times) {
    if (times == 0) return;

    buzzerIsActive = true;
    buzzerStart = millis();
    buzzerEnd = buzzerStart + (times * BUZZER_DURATION * 2);  // Multiply by 2 to allow for gap between beeps
    if (digitalRead(LOUD_PIN) && digitalRead(QUIET_PIN)) {
        // Central switch (mute)
        buzzerIsActive = false;
    } else if (digitalRead(QUIET_PIN)) {
        analogWrite(BUZZER_PIN, QUIET);
    } else {
        analogWrite(BUZZER_PIN, LOUD);
    }
}

/**
 * Based on the colour in the current state, illuminate the corresponding LED grid with red, amber, or green.
 */
void updateColourFromState() {
    switch(state.colour) {
        case RED:
            displayRedLight(leds);
            clearAmberLight(leds);
            clearGreenLight(leds);
            ledsDirty = true;
            break;

        case AMBER:
            clearRedLight(leds);
            displayAmberLight(leds);
            clearGreenLight(leds);
            ledsDirty = true;
            break;

        case GREEN:
            clearRedLight(leds);
            clearAmberLight(leds);
            displayGreenLight(leds);
            ledsDirty = true;
            break;

        default:
            break;
    }
}

/**
 * Based on the detail setting in the current state, display A/B, C/D, or nothing on the detail 7-segs.
 */
void updateDetailFromState() {
    switch(state.detail) {
        case DETAIL_OFF:
            clearAC(leds);
            clearBD(leds);
            ledsDirty = true;
            break;

        case DETAIL_AB:
            displayA(leds);
            displayB(leds);
            ledsDirty = true;
            break;

        case DETAIL_CD:
            displayC(leds);
            displayD(leds);
            ledsDirty = true;
            break;

        default:
            break;
    }
}

/**
 * Enter into a blank state, where nothing is displayed except for the red grid.
 */
void enterBlankState() {
    clearAC(leds);
    clearBD(leds);
    clearNumber(leds);
    state.colour = RED;
    updateColourFromState();
    ledsDirty = true;
}

/**
 * Enter into an idle state, where detail, time, and the red grid are showing.
 */
void enterIdleState() {
    state.colour = RED;
    updateColourFromState();
    displayNumber(leds, state.time);

    switch (state.detail) {
        case DETAIL_OFF:
            clearAC(leds);
            clearBD(leds);
            ledsDirty = true;
            break;

        case DETAIL_AB:
            displayA(leds);
            displayB(leds);
            ledsDirty = true;
            break;

        case DETAIL_CD:
            displayC(leds);
            displayD(leds);
            ledsDirty = true;
            break;

        default:
            break;
    }
}

/**
 * Updates the time being displayed if a countdown is running.
 * If the countdown should finish, beeps 3 times and enters a blank state.
 */
void handleCountDown() {
    if (state.countdown) {
        unsigned long now = millis();
        if (now - 1000 >= startTime) {


            Serial.println(state.time);


            if (state.time > 1) {
                displayNumber(leds, --state.time);
                ledsDirty = true;
                startTime = now;
                return;
            }

            Serial.println("about to reach 0");

            // About to reach 0 seconds on countdown
            beep(state.endNumBeeps);
            if (!state.countdownContinues) {
                state.countdown = false;
                enterBlankState();
                return;
            }
            state.countdownContinues = false;


//            // About to reach 0 seconds on countdown
//            state.countdown = false;
//            state.time = 0;  // TODO: Remember max time when countdown starts, to return once done?
//            beep(state.endNumBeeps);
//            if (!state.countdownContinues) {
//                enterBlankState();
//                return;
//            }
//            state.countdownContinues = false;
//            ledsDirty = true;

            // In matchplay, when one light runs out of time and there are more arrows to shoot it should not beep
            // but if all arrows are shot it should beep
        }
    }
}

/**
 * Toggles the buzzer on/off every BUZZER_DURATION ms, if the buzzer should be making sound.
 * Once the buzzer has finished, enters the idle state.
 */
void handleBuzzer() {
    if (buzzerIsActive) {
        unsigned long now = millis();
        if (buzzerEnd > now) {
            // Toggle the buzzer on/off every BUZZER_DURATION ms
            if ((now - buzzerStart) % (BUZZER_DURATION * 2) < BUZZER_DURATION) {
                // Consideration here only needs to be given to the loud/quiet conditions and not the mute condition
                // as if the unit should be muted it should not pass the first if condition
                if (digitalRead(LOUD_PIN)) {
                    analogWrite(BUZZER_PIN, LOUD);
                } else {
                    analogWrite(BUZZER_PIN, QUIET);
                }
            } else {
                analogWrite(BUZZER_PIN, MUTE);
            }
        } else {
            buzzerIsActive = false;
            analogWrite(BUZZER_PIN, MUTE);
            enterIdleState();
        }
    }
}

/**
 * Configures the brightness of the LEDs based on the physical switch.
 */
void configureBrightness() {
    int brightness = digitalRead(BRIGHTNESS_PIN);
    if (brightness == oldBrightness) return;

    if (brightness == HIGH) {
        FastLED.setBrightness(HIGH_BRIGHTNESS);
    } else {
        FastLED.setBrightness(LOW_BRIGHTNESS);
    }
    oldBrightness = brightness;
    ledsDirty = true;
}

/**
 *
 */
void configureMatchplayMode() {
    if (digitalRead(MATCHPLAY_PIN) == HIGH) {
        matchplayMode = 2;  // CD/R
    } else {
        matchplayMode = 1;  // AB/L
    }
}

/**
 * Runtime loop of the Arduino.
 */
void loop() {
    // While there is serial data available
    while (Serial.available()) {
        // Read any data available into the buffer
        buffer.writeByte(Serial.read());

        // If we have 5 bytes, try and parse the header
        if (buffer.getSize() == 5) {
            for (int i = 0; i < 4; i++) {
                byte b = buffer.peekByte(i);
                if (b != HEADER[i]) {
#ifdef DEBUG_LOGGING
                    printBuffer("Header failed: ", buffer);
#endif
                    // If the header doesnt match, remove the leading byte and try again
                    buffer.setReaderIndex(1);
                    buffer.take();
                    goto cont;
                }
            }
            // Header validated, grab the size, and exit the loop
            expectedSize = buffer.peekByte(4);
#ifdef VERBOSE_DEBUG_LOGGING
            Serial.println("Valid packet. Expected size: " + String(expectedSize));
#endif
        }

        if (expectedSize != -1 && buffer.getSize() == expectedSize) {
#ifdef VERBOSE_DEBUG_LOGGING
            printBuffer("Full packet: ", buffer);
#endif
            buffer.skip(5);
            unsigned int expectedChecksum = buffer.readUInt();

            unsigned int checksum = makeChecksum(buffer);
#ifdef VERBOSE_DEBUG_LOGGING
            Serial.println("Expected checksum: " + String(expectedChecksum) + ", Calculated: " + String(checksum));
#endif
            if (expectedChecksum == checksum) {
#ifdef VERBOSE_DEBUG_LOGGING
                Serial.println("Checksum validated.");
#endif
                handlePacket(buffer);
            }

            // Clear buffer in case of unread bytes
            buffer.clear();
            expectedSize = -1;
        }
        cont:;
    }

    handleCountDown();
    handleBuzzer();
    configureBrightness();
    configureMatchplayMode();

    // Only send FastLED updates if the LEDs were changed AND no more packet data is expected.
    // Too many calls to FastLED.show() fills up the serial buffer and causes packet read failure.
    if (ledsDirty && expectedSize == -1) {
        FastLED.show();
        ledsDirty = false;
    }
}

/**
 * Updates the internal state from the received byte buffer, and handles the inputs.
 *
 * @param buf A ByteBuf containing the packet data sent by the Android app.
 */
void handlePacket(ByteBuf &buf) {
    int data = buf.readInt();

    bool oldCountdownContinues = state.countdownContinues;
    bool oldCountdown = state.countdown;
    bool oldDetail = state.detail;
    byte oldColour = state.colour;
    bool oldTimeEnabled = state.timeEnabled;
    bool oldTime = state.time;

    // Update internal state
    int detail = data >> 3 & 0x3;
    if (isBitSet(6, data) && detail != matchplayMode) {
        // If in matchplay mode and detail does not indicate matchplay mode, ignore the packet
        return;
    }

    state.countdownContinues = isBitSet(9, data);
    state.lastEnd = isBitSet(8, data);
    state.countdown = isBitSet(5, data);
    // 00 = DETAIL_OFF, 01 = DETAIL_AB, 10 = DETAIL_CD
    state.detail = detail;
    // 00 = RED, 01 = AMBER, 10 = GREEN
    state.colour = data >> 1 & 0x3;
    state.timeEnabled = isBitSet(0, data);
    state.time = buf.readInt();
    state.startNumBeeps = buf.readInt();
    state.endNumBeeps = buf.readInt();

#ifdef DEBUG_LOGGING
    Serial.print(data, BIN);
    Serial.println();

    Serial.println("Updated State:");
    Serial.println("  Countdown continues:  " + String(state.countdownContinues));
    Serial.println("  Last end (matchplay): " + String(state.lastEnd));
    Serial.println("  Should count down:    " + String(state.countdown));
    Serial.println("  Detail:               " + String(state.detail == 0 ? "None" : state.detail == 1 ? "A/B" : "C/D"));
    Serial.println("  Colour:               " + String(state.colour == 0 ? "Red" : state.colour == 1 ? "Amber" : "Green"));
    Serial.println("  Should show time:     " + String(state.timeEnabled));
    Serial.println("  Time value:           " + String(state.time));
#endif

    if (oldCountdownContinues != state.countdownContinues && oldCountdownContinues) {
        // Last instruction says countdown continues, new one says countdown does not
        // => ignore new packet countdownContinues to allow for lag between light and controller
        state.countdownContinues = true;
    }

    if (isBitSet(7, data)) {
        //TODO: Properly handle emergency stop (stop countdown)

        // Emergency stop pushed
        beep(5);
        enterBlankState();
        return;
    }

    if ((oldTime != state.time && state.timeEnabled) || (!oldTimeEnabled && state.timeEnabled)) {
        displayNumber(leds, state.time);
        ledsDirty = true;
    }

    if (oldTimeEnabled && !state.timeEnabled) {
        clearNumber(leds);
        ledsDirty = true;
    }

    if (oldColour != state.colour) {
        updateColourFromState();
    }

    if (oldDetail != state.detail) {
        updateDetailFromState();
    }

    if (oldCountdown != state.countdown) {
        if (state.countdown) {
            startTime = millis();
            state.time -= 1;
            beep(state.startNumBeeps);
            displayNumber(leds, state.time);
            ledsDirty = true;
        } else {
            beep(state.endNumBeeps);
            enterBlankState();
        }
    }
}

/**
 * Debugging method to print the received byte buffer.
 *
 * @param prefix Message string indicating the reason for error.
 * @param buf    A ByteBuf containing the packet data sent by the Android app.
 */
void printBuffer(const String &prefix, ByteBuf &buf) {
    Serial.print(prefix);
    for (int i = 0; i < buf.getSize(); i++) {
        byte b = buf.peekByte(i);
        if (i) {
            Serial.print(' ');
        }
        Serial.print(HEX_CHARS[(b >> 4) & 0x0F]);
        Serial.print(HEX_CHARS[b & 0x0F]);
    }
    Serial.println();
}

/**
 * Calculate a basic checksum for the given byte buffer.
 *
 * @param buf A ByteBuf containing the packet data sent by the Android app.
 * @return    The checksum for the given packet.
 */
unsigned int makeChecksum(ByteBuf &buf) {
    unsigned int c = 0;
    for (int i = 0; i < buf.getSize(); i++) {
        c += buf.peekByte(i);
    }
    return c;
}
