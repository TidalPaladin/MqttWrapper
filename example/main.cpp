#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <functional>
#include "MqttTopic.h"


void setup() {
    
    Serial.begin(115200);
    WiFi.begin("SSID", "PASS");
    
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

    MqttTopic::setServer("SERVER", 1883);
}

void loop() {
    MqttTopic::loop(); 
}