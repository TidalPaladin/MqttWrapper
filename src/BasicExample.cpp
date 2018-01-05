#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <functional>
#include <Ticker.h>
#include "MqttTopic.h"

#define SSID        "Bowser"
#define PASSWORD    "Bowser1993"
#define SERVER      "192.168.1.237"

Ticker publish_data;

void setup() {
    
    Serial.begin(115200);
    WiFi.begin(SSID, PASSWORD);
    
    // Create objects for each desired topic
    static MqttTopic light_state("desklamp/state");    
    static MqttTopic light_command("desklamp/command");
    static MqttTopic numeric_state("numeric/topic");   

    // Attach callbacks to be run for "ON" and "OFF" payloads
    light_command.callback("ON", []() {
        Serial.println("ON");
        light_state.publish("ON");
    });
    light_command.callback("OFF", []() {
        Serial.println("OFF");
        light_state.publish("OFF");
    });

    // Or attach a callback to run on any payload
    light_command.callback([](mqtt_state_t state){
        Serial.println(state.c_str());
        light_state.publish(state);
    });

    // Identify the MQTT server
    MqttTopic::setServer(SERVER, 1883);

    // Publish system time every five seconds
    publish_data.attach_ms(5000, [](){
        numeric_state.publish(millis());
    });
}

void loop() {
    MqttTopic::loop(); 
}