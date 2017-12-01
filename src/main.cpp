#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <functional>
#include "MqttTopic.h"


template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }



void setup() {
    
    Serial.begin(115200);
    WiFi.begin("Bowser", "Bowser1993");
    delay(2000);
    Serial.println("Connected"); 

    static MqttTopic light_state("test");    
    static MqttTopic light_command("test2");   

    light_command.callback("ON", []() {
        Serial.println("ON");
        light_state.publish("ON");
    });
    light_command.callback("OFF", []() {
        Serial.println("OFF");
        light_state.publish("OFF");
    });

    MqttTopic::setServer("192.168.1.238", 1883);
}

void loop() {
    MqttTopic::loop(); 
}