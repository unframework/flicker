#include <SimplexNoise.h>

void setup() {
    // initialize pins as output
    pinMode(2, OUTPUT);
}

int level = 10;

const int cycleLength = 5000; // in microseconds; 5000us is 200Hz
unsigned long nextCycleStart = 0;
bool currentState = false;

SimplexNoise sn;

void loop() {
    unsigned long currentMicros = micros();

    if (currentState) {
        // downstroke
        if (nextCycleStart + level <= currentMicros) {
            // prep next timeout
            nextCycleStart += cycleLength;

            // flip pin
            digitalWrite(2, LOW);
            currentState = false;

            // change PWM level as needed
            double currentSeconds = currentMicros / 1000000.0;

            // slow overall brightness change
            double baseGlow = 0.5 + 0.5 * sn.noise(currentSeconds * 0.03, 0);
            baseGlow *= baseGlow; // exaggerate the dips in brightness

            // simple frequent oscillation
            double baseBreath = 0.5 + 0.5 * sn.noise(currentSeconds * 0.5, 0.5);

            // amplify and clamp the throttle noise for a clearer mode
            double flickerThrottleRaw = sn.noise(currentSeconds * 0.5, 1.0);
            double flickerThrottle = min(1.0, 3.0 * max(0.0, flickerThrottleRaw - 1.0 + 0.5) / 0.5);
            double flicker = 0.5 + 0.3 * sn.noise(currentSeconds * 8.0, 2.0) + 0.2 * sn.noise(currentSeconds * 14.0, 2.0);

            // mix up the proportioned factors
            double total = baseGlow * (1.0 - baseBreath * 0.2) * (1.0 - flickerThrottle * flicker * 0.5);
            level = total * cycleLength;
        }
    } else {
        // upstroke
        if (nextCycleStart <= currentMicros) {
            // flip pin with no actions yet
            digitalWrite(2, HIGH);
            currentState = true;
        }
    }
}
