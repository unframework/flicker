
void setup() {
    // initialize pins as output
    pinMode(2, OUTPUT);
}

int level = 1;

const int cycleLength = 15;
unsigned long nextCycleStart = 0;
bool currentState = false;

void loop() {
    unsigned long currentMillis = millis();

    if (currentState) {
        // downstroke
        if (nextCycleStart + level <= currentMillis) {
            // prep next timeout
            nextCycleStart += cycleLength;

            // flip pin
            digitalWrite(2, LOW);
            currentState = false;

            // change PWM level as needed
            level = (currentMillis / 100) % 16;
        }
    } else {
        // upstroke
        if (nextCycleStart <= currentMillis) {
            // flip pin with no actions yet
            digitalWrite(2, HIGH);
            currentState = true;
        }
    }
}
