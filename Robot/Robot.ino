#include "EspNow.h"
#include "Motor.h"
#include "Addr.h"

/*-----------------------------*/
/*--EspNow send/recv func&var--*/
/*-----------------------------*/
EspNow* espNow;
typedef struct send_struct{
    int id;
    int stateID;
    int speedL; // send: batt
    int speedR;
}Send_struct;
Send_struct getData = {0,0,0,0};
Send_struct sendData ={5,0,0,0};
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    char macStr[18];
    Serial.print("Packet to: ");
    // Copies the sender mac address to a string
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print(macStr);
    Serial.print(" send status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    // Serial.println("Receive!");
    Send_struct buf;
    memcpy(&buf, incomingData, sizeof(buf));
    getData = buf;
}

/*-----------------------------*/
/*---------- Motors -----------*/
/*-----------------------------*/
Motor* motorL;
Motor* motorR;
// [0]:L [1]:R
const int motorL_pin[2] = {D2,D3};
const int motorR_pin[2] = {D1,D8};

/*-----------------------------*/
/*---------- Switches ---------*/
/*-----------------------------*/
const int sw[2] = {D10,D9};
const int bufSize = 5;
int swState_L[bufSize] = {0,0,0,0,0};
int swState_R[bufSize] = {0,0,0,0,0};
int swFlag[2] = {0,0};
double swStateUpdate(int state[], int pin){
    double sum = 0;
    for (int i=bufSize-1; i>0; i-=1) {
        state[i] = state[i-1];
        sum += state[i];
    }
    state[0] = digitalRead(pin);
    sum += state[0];
    return sum/bufSize;
}

/*-----------------------------*/
/*---------- Battery -----------*/
/*-----------------------------*/
const int Batt = A0;
float battRead(){ // read battery status
    uint32_t Vbatt = 0;
    for(int i = 0; i < 16; i++) {
        Vbatt = Vbatt + analogReadMilliVolts(Batt); // ADC with correction   
    }
    float Vbattf = 2 * Vbatt / 16 / 1000.0;     // attenuation ratio 1/2, mV --> V
    return Vbattf;
}

/*-----------------------------*/
/*----------- Codes -----------*/
/*-----------------------------*/
void setup() {
    Serial.begin(9600);
    espNow = new EspNow();
    // espNow->Init<esp_now_recv_cb_t>(RECEIVER, OnDataRecv);
    espNow->Init(MEMBER, OnDataSent, OnDataRecv);
    if(!espNow->addPeer(macAddr[0])){
        Serial.println("Failed to add peer");
        return;
    }
    
    motorL = new Motor(motorL_pin[0], motorL_pin[1]);
    motorR = new Motor(motorR_pin[0], motorR_pin[1]);
    motorL->move(0);
    motorR->move(0);

    pinMode(sw[0], INPUT_PULLUP);
    pinMode(sw[1], INPUT_PULLUP);
    int state_L = digitalRead(sw[0]);
    int state_R = digitalRead(sw[1]);
    for (int i=0; i<5; i+=1) {
        swState_L[i] = state_L;
        swState_R[i] = state_R;
    }
    
    pinMode(Batt, INPUT);
    float battStatus = battRead();
    sendData.speedL =(int)(battStatus*100);
    espNow->send(macAddr[0], sendData);

    delay(5000);
}

int sendBatt = 0;

void loop() {
    if (sendBatt == 5000) {
        sendData.speedL = (int)(battRead()*100);
        // Serial.println((float)sendData.speedL/100);
        espNow->send(macAddr[0], sendData);
        sendBatt = 0;
    } sendBatt += 1;

    int nowState = getData.stateID;
    int nowSpeedL= getData.speedL;
    int nowSpeedR= getData.speedR;
    switch (nowState) {
        case 0: // Stop
            motorL->move();
            motorR->move();
            break;
        case 1: // Straight
            motorL->move(nowSpeedL);
            motorR->move(nowSpeedL);
            break;
        case 2: // Left
            motorL->move(-50);
            motorR->move(255);
            break;
        case 3: // Right
            motorL->move(255);
            motorR->move(-50);
            break;
        default: // Free
            motorL->move(nowSpeedL);
            motorR->move(nowSpeedR);
            break;
    }

    Serial.printf("%d, %3d, %3d\n", nowState, nowSpeedL, nowSpeedR);

    delay(5);
}
