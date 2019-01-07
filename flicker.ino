#include <SimplexNoise.h>

void setup() {
    // initialize pins as output
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
}

const unsigned long cycleLength = 5000; // in microseconds; 5000us is 200Hz

SimplexNoise sn;

class PinState {
    int pin;
    double slice;
    int level = 100;
    unsigned long nextCycleStart;
    bool currentState = false;

public:
    PinState(int pin, double slice);
    void update(unsigned long currentMicros);
};

PinState::PinState(int pin, double slice) {
    this->pin = pin;
    this->slice = slice;

    nextCycleStart = slice * 100; // sort of randomize phase
}

void PinState::update(unsigned long currentMicros) {
    if (currentState) {
        // downstroke
        if (nextCycleStart + level <= currentMicros) {
            // prep next timeout
            nextCycleStart += cycleLength;

            // flip pin
            if (pin == 2) digitalWrite(pin, LOW);
            currentState = false;

            // change PWM level as needed
            double currentSeconds = currentMicros / 1000000.0;

            // slow overall brightness change
            double baseGlow = 0.5 + 0.5 * sn.noise(currentSeconds * 0.03, slice + 0);
            baseGlow *= baseGlow; // exaggerate the dips in brightness

            // simple frequent oscillation
            double baseBreath = 0.5 + 0.5 * sn.noise(currentSeconds * 0.5, slice + 0.5);

            // amplify and clamp the throttle noise for a clearer mode
            double flickerThrottleRaw = sn.noise(currentSeconds * 0.5, slice + 1.0);
            double flickerThrottle = min(1.0, 3.0 * max(0.0, flickerThrottleRaw - 1.0 + 0.5) / 0.5);
            double flicker = 0.5 + 0.3 * sn.noise(currentSeconds * 8.0, slice + 2.0) + 0.2 * sn.noise(currentSeconds * 14.0, slice + 2.0);

            // mix up the proportioned factors
            double total = baseGlow * (1.0 - baseBreath * 0.2) * (1.0 - flickerThrottle * flicker * 0.5);
            level = total * cycleLength;
        }
    } else {
        // upstroke
        if (nextCycleStart <= currentMicros) {
            // flip pin with no actions yet
            if (pin == 2) digitalWrite(pin, HIGH);
            currentState = true;
        }
    }
}

class PinState a(2, 0.0), b(3, 10.0), c(4, 20.0), d(5, 30.0);

void loop() {
    unsigned long currentMicros = micros();

    a.update(currentMicros);
    b.update(currentMicros);
    c.update(currentMicros);
    d.update(currentMicros);
}
