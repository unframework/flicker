
void setup() {
    // initialize pins as output
    pinMode(2, OUTPUT);
}

int level = 10;

const int cycleLength = 5000; // in microseconds; 5000us is 200Hz
unsigned long nextCycleStart = 0;
bool currentState = false;

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
            level = ((currentMicros / 10000) % 100) * cycleLength / 100;
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
