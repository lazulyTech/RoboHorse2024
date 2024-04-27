#include <M5StickCPlus.h>
#include "EspNow.h"
#include "Addr.h"

/*-----------------------------*/
/*--EspNow send/recv func&var--*/
/*-----------------------------*/
EspNow* espNow;
typedef struct send_struct{
    int id;
    int stateID;
    int speedL;
    int speedR;
}Send_struct;
Send_struct getData = {0,0,0,0};
Send_struct sendData = {0,0,128,128};
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
    Send_struct buf;
    memcpy(&buf, incomingData, sizeof(buf));
    // if(buf.id == 0 || buf.id == urc::teamNumber) urc::myData = buf;
    if (buf.id == 5) getData = buf;
}

int lastStatus = 0;

void Send(){
    // if (sendData.stateID != lastStatus) {
        espNow->send(macAddr[0], sendData);
    // }
    lastStatus = sendData.stateID;
}

float IMUpreset[3] = {0,0,0};
void getIMU(float* pitch, float* roll, float* yaw){
    M5.IMU.getAhrsData(pitch, roll, yaw);
    *pitch -= IMUpreset[0];
    *roll -= IMUpreset[1];
    *yaw -= IMUpreset[2];
}

void setup() {
    M5.begin();
    M5.Lcd.begin();
    M5.Lcd.setRotation(1);
    M5.Lcd.setTextWrap(false);

    M5.Lcd.fillScreen(BLACK);           // 画面背景(指定色で画面全体を塗りつぶす。表示を更新する場合にも使用)
    M5.Lcd.setTextColor(ORANGE, BLACK); // テキスト色(文字色, 文字背景)
    M5.Lcd.setTextFont(7);              // フォント(フォント番号：0,2,4,6,7,8の中から指定)
    M5.Lcd.setCursor(0, 0);             // テキスト表示座標(x座標, y座標)
    M5.Lcd.setCursor(0, 0, 2);          // テキスト表示座標(x座標, y座標, フォント番号)※フォント指定も可
    M5.Lcd.setTextSize(0);

    M5.Imu.Init();
    for (int i=0; i>50; i++) {
        M5.update();
        M5.IMU.getAhrsData(&IMUpreset[0], &IMUpreset[1], &IMUpreset[2]);
        delay(10);
    }
    M5.update();
    M5.IMU.getAhrsData(&IMUpreset[0], &IMUpreset[1], &IMUpreset[2]);
    Serial.begin(9600);
    // put your setup code here, to run once:
    espNow = new EspNow();
    espNow->Init(MEMBER, OnDataSent, OnDataRecv);
    if(!espNow->addPeer(macAddr[0])){
        Serial.println("Failed to add peer");
        return;
    }
    M5.update();
    while(M5.BtnB.isReleased()) {
        M5.update();
        M5.Lcd.setCursor(0, 0, 7);
        M5.Lcd.printf("|%05.2f|\n", (float)getData.speedL/100);
        M5.Lcd.printf("%d:|%03d|\n", sendData.stateID , 0);
        delay(100);
    }
    sendData.stateID = 1;
    espNow->send(macAddr[0], sendData);
}
bool isEmergency = 0;
int speed = sendData.speedL;
void loop() {
    M5.update();
    if (M5.Axp.GetBtnPress() != 1 && !isEmergency){
        // 通常操作
        float acc[3];
        M5.IMU.getAccelData(&acc[0], &acc[1], &acc[2]);
        float imu[3];
        getIMU(&imu[0], &imu[1], &imu[2]);
        Serial.printf("%f\t%f\t%f\n", imu[0], imu[1], imu[2]);

        // 速度変化: imu[0]
        if (imu[0] > 20) {
            //加速
            speed += (imu[0]-20)/10;
        }
        else if (imu[0] < -35) {
            //減速
            speed += (imu[0]+35)/20;
        }
        if (speed > 255) speed = 255;
        if (speed < 0) speed = 0;

        // 方向変化: imu[1]
        int start = 15;
        int halfRange = (80-start)/2;
        if (imu[1] > start) {
            //右回り
            sendData.speedL = speed;
            sendData.speedR = speed*(-imu[1]+start+halfRange)/halfRange;
        }
        else if (imu[1] < -start) {
            //左回り
            sendData.speedL = speed*(imu[1]+start+halfRange)/halfRange;
            sendData.speedR = speed;
        } else {
            sendData.speedL = speed;
            sendData.speedR = speed;
        }


        // int diff = fabs(acc[0]);
        // if (M5.BtnB.isReleased()) sendData.speedL += diff*2;
        // else sendData.speedL -= diff+1;

        if (M5.BtnA.isPressed()) {
            sendData.stateID = 1;
            sendData.speedL = -128;
            sendData.speedR = -128;
            // sendData.speedR = sendData.speedL;
            // sendData.speedL *= 0.7;
            if (M5.BtnB.isPressed()) {
                sendData.stateID = 3;
            }
        } else {
            sendData.stateID = 4;
        }
        
        // if (sendData.stateID == 1 && diff != 0) Send();//espNow->send(macAddr[0], sendData);
    } else {
        // 非常停止 -> steteID=0, 停止
        isEmergency = 1;
        sendData.stateID = 0;
        sendData.speedL = 0;
        sendData.speedR = 0;
        if (M5.BtnA.pressedFor(1000)) isEmergency = 0;
    }

    // int min = 0;
    // if (sendData.speedL < min) sendData.speedL = min;
    Send();
    
    M5.Lcd.setCursor(0, 0, 7);
    M5.Lcd.printf("|%05.2f|\n", (float)getData.speedL/100);
    M5.Lcd.printf("%d:|%03d|\n", sendData.stateID ,  sendData.speedL);
    delay(25);
    if (isEmergency) delay(750);
}
