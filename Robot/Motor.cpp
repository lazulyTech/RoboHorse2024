#include "esp32-hal.h"
#include "Motor.h"
#include <Arduino.h>

void Motor::move(int speed){
    // int min = 128;
    // if (speed > 0 && speed <= min)  speed = min;
    // if (speed >= -min && speed < 0) speed = -min;
    if (speed > 255) speed = 255;
    if (speed < -255) speed = -255;
    if (speed >= 0) {
        analogWrite(pinL, speed);
        analogWrite(pinR, 0);
    } else {
        analogWrite(pinL, 0);
        analogWrite(pinR, -speed);
    }
}
void Motor::stop(){
    analogWrite(pinL, 255);
    analogWrite(pinR, 255);
}