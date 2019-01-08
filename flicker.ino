#include <TimerOne.h>
#include <SimplexNoise.h>

SimplexNoise sn;

const unsigned long cycleLength = 6000; // in microseconds; 5000us is 200Hz
const unsigned long pollPeriod = 100; // in microseconds
unsigned long currentMicros = 0;

void setup() {
    // initialize pins as output
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);

    Timer1.initialize(pollPeriod); // allow for some precision
    Timer1.attachInterrupt(renderAll);

    sn.init();
}

class PinState {
    int pin;
    double yOffset;
    unsigned long level = cycleLength / 2;
    unsigned long nextCycleStart;
    bool currentState = false;

public:
    PinState(int pin, double phase);
    void computeLevel(double currentSeconds, double flickerMultiplier);
    void setLevel(unsigned long level);
    void render();
};

PinState::PinState(int pin, double phase) {
    this->pin = pin;
    this->yOffset = phase * 100.0;

    nextCycleStart = phase * cycleLength; // stagger computation/strobe
}

void PinState::setLevel(unsigned long level) {
    noInterrupts();
    this->level = level;
    interrupts();
}

void PinState::computeLevel(double currentSeconds, double flickerMultiplier) {
    // slow overall brightness change
    double baseGlow = 0.5 + 0.5 * sn.noise(currentSeconds * 0.03, yOffset + 0);
    baseGlow *= baseGlow; // exaggerate the dips in brightness

    // simple frequent oscillation
    double baseBreath = 0.5 + 0.5 * sn.noise(currentSeconds * 0.5, yOffset + 0.5);

    // mix up the proportioned factors
    double total = baseGlow * (1.0 - baseBreath * 0.5) * flickerMultiplier;
    setLevel(total * cycleLength);
}

void PinState::render() {
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
    noInterrupts();
    unsigned long currentMicrosSnapshot = currentMicros;
    interrupts();

    // change PWM level as needed
    double currentSeconds = currentMicrosSnapshot / 1000000.0;

    // amplify and clamp the throttle noise for a clearer mode
    // the flicker is synchronized across all phases
    double flickerThrottleRaw = sn.noise(currentSeconds * 0.25, 1.0);
    double flickerThrottle = min(1.0, 3.0 * max(0.0, flickerThrottleRaw - 1.0 + 0.5) / 0.5);
    double flicker = 0.5 + 0.3 * sn.noise(currentSeconds * 8.0, 2.0) + 0.2 * sn.noise(currentSeconds * 14.0, 2.0);
    double flickerMultiplier = (1.0 - flickerThrottle * flicker * 0.8);

    a.computeLevel(currentSeconds, flickerMultiplier);
    b.computeLevel(currentSeconds, flickerMultiplier);
    c.computeLevel(currentSeconds, flickerMultiplier);
    d.computeLevel(currentSeconds, flickerMultiplier);

    delay(20);
}

void renderAll() {
    currentMicros += pollPeriod;

    a.render();
    b.render();
    c.render();
    d.render();
}
