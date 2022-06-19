class Gercon {
private:
    int pin = 0;
    bool lastState;
    bool waiting = false;
    unsigned long waitTime = 0;
public:
    Gercon(int pinIn) {
        pin = pinIn;
        pinMode(pin, INPUT);
        lastState = digitalRead(pin);
    }

    void tick(){
        bool currentState = digitalRead(pin);
        if (!waiting) {
            if (currentState != lastState) {
                waiting = true;
                waitTime = millis();
            }
        } else {
            if (millis() - waitTime > 100) {
                waiting = false;
                lastState = currentState;
            }
        }
    }

    bool getState(){
        return !lastState;
    }
};