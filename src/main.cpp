//#define BLINKER_PRINT Serial
#define BLINKER_WITH_SSL
#define BLINKER_WIFI
#define BLINKER_MIOT_OUTLET
#include <Blinker.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <Ticker.h>

const char *mqtt_broker = "1.14.34.20";
const char *topic = "esp8266/door";
const char *mqtt_username = "door";
const char *mqtt_password = "265265265YY";
const int mqtt_port = 1883;
const char auth[] = "ceee0e4a1164";
const char ssid[] = "WIFI_2.4G";
const char pswd[] = "265265265YY";
const int openAngle = 17;
String client_id = "esp8266-client-"+String(WiFi.macAddress());
// 新建组件对象
BlinkerButton Button((char*)"btn-switch");
WiFiClient espClient;
PubSubClient mqttClient(espClient);
// 创建定时器
Ticker ticker;
//Ticker servoTicker;


void reConnect(){
    if (!Blinker.connected()){
        Blinker.begin(auth, ssid, pswd);
    }
    if (!mqttClient.connected()){
        mqttClient.connect(client_id.c_str(), mqtt_username, mqtt_password);
    }
}

void servoRun(int angle){
    if (angle<5){
        angle = 5;
    }
    if (angle>25){
        angle = 25;
    }
    analogWrite(D4,angle);
    delay(1700);
    analogWrite(D4,5);
    delay(1000);
    analogWrite(D4,0);
    digitalWrite(LED_BUILTIN, HIGH);
}

void button_callback(const String & state) {
//    BLINKER_LOG("get button state: ", state);
    servoRun(openAngle);
}
void miotPowerState(const String & state)
{
    if (state == BLINKER_CMD_ON) {
        BlinkerMIOT.powerState("on");
        BlinkerMIOT.print();
        servoRun(openAngle);
    }
    else if (state == BLINKER_CMD_OFF) {
        BlinkerMIOT.powerState("off");
        BlinkerMIOT.print();
        servoRun(openAngle);
    }
}
void mqtt_callback(char *_topic, byte *payload, unsigned int length) {
//    Serial.print("Message arrived in topic: ");
//    Serial.println(_topic);
//    Serial.print("Message:");
    String message;
    for (int i = 0; i < length; i++) {
        message = message + (char) payload[i];  // convert *byte to string
    }
//    Serial.print(message);

    if (message == "on"){ servoRun(openAngle); }   // LED on
    else{
        int i = message.toInt();
        servoRun(i);
    }

//    Serial.println();
//    Serial.println("-----------------------");
}


void setup() {
    // 初始化串口
//    Serial.begin(115200);

//#if defined(BLINKER_PRINT)
//    BLINKER_DEBUG.stream(BLINKER_PRINT);
//#endif

    // 初始化有LED的IO
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    // 初始化blinker
    Blinker.begin(auth, ssid, pswd);
    Button.attach(button_callback);
    BlinkerMIOT.attachPowerState(miotPowerState);
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setCallback(mqtt_callback);
    while (!mqttClient.connected()) {
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (mqttClient.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
            mqttClient.publish(topic, "esp8266 connected");
            mqttClient.subscribe(topic);
            digitalWrite(LED_BUILTIN, HIGH);//off led
        } else {
            Serial.print("failed with state ");
            Serial.print(mqttClient.state());
            delay(2000);
        }
    }
    analogWriteFreq(50);
    analogWriteRange(200);
    analogWrite(D4,5);
    delay(1000);
    analogWrite(D4,0);
    ticker.attach(600,reConnect);
    // 休眠
    wifi_set_sleep_type(LIGHT_SLEEP_T);
//    digitalWrite(D2,HIGH);
//    digitalWrite(D3,LOW);
}

void loop() {
    Blinker.run();
    mqttClient.loop();
}