#pragma once

class Motor{
private:
    int pinL;
    int pinR;
public:
    Motor(int _pinL, int _pinR){pinL = _pinL; pinR = _pinR;}
    void move(int speed=0);
    void stop();
};