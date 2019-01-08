#include <SimplexNoise.h>

SimplexNoise sn;

void setup() {
    // initialize pins as output
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);

    sn.init();
}

const unsigned long cycleLength = 5000; // in microseconds; 5000us is 200Hz

class PinState {
    int pin;
    double yOffset;
    int level = 100;
    unsigned long nextCycleStart;
    bool currentState = false;

public:
    PinState(int pin, double phase);
    void computeLevel();
    void render(unsigned long currentMicros);
};

PinState::PinState(int pin, double phase) {
    this->pin = pin;
    this->yOffset = phase * 100.0;

    nextCycleStart = phase * cycleLength; // stagger computation/strobe
}

void PinState::computeLevel() {
    // change PWM level as needed
    double currentSeconds = micros() / 1000000.0;

    // slow overall brightness change
    double baseGlow = 0.5 + 0.5 * sn.noise(currentSeconds * 0.03, yOffset + 0);
    baseGlow *= baseGlow; // exaggerate the dips in brightness

    renderAll();

    // simple frequent oscillation
    double baseBreath = 0.5 + 0.5 * sn.noise(currentSeconds * 0.5, yOffset + 0.5);

    renderAll();

    // amplify and clamp the throttle noise for a clearer mode
    double flickerThrottleRaw = sn.noise(currentSeconds * 0.5, yOffset + 1.0);
    renderAll();
    double flickerThrottle = min(1.0, 3.0 * max(0.0, flickerThrottleRaw - 1.0 + 0.5) / 0.5);
    double flicker = 0.5 + 0.3 * sn.noise(currentSeconds * 8.0, yOffset + 2.0) + 0.2 * sn.noise(currentSeconds * 14.0, yOffset + 2.0);
    renderAll();

    // mix up the proportioned factors
    double total = baseGlow * (1.0 - baseBreath * 0.2) * (1.0 - flickerThrottle * flicker * 0.5);
    level = total * cycleLength;
}

void PinState::render(unsigned long currentMicros) {
    if (currentState) {
        // downstroke
        if (nextCycleStart + level <= currentMicros) {
            // prep next timeout
            nextCycleStart += cycleLength;

            // flip pin
            digitalWrite(pin, LOW);
            currentState = false;
        }
    } else {
        // upstroke
        if (nextCycleStart <= currentMicros) {
            // flip pin with no actions yet
            digitalWrite(pin, HIGH);
            currentState = true;
        }
    }
}

class PinState a(2, 0.0), b(3, 0.25), c(4, 0.5), d(5, 0.75);

void loop() {
    a.computeLevel();
    // b.computeLevel();
    // c.computeLevel();
    // d.computeLevel();
}

void renderAll() {
    unsigned long currentMicros = micros();

    a.render(currentMicros);
    b.render(currentMicros);
    c.render(currentMicros);
    d.render(currentMicros);
}
