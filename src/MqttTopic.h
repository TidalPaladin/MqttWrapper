/**
 * MqttTopic.h
 * Scott Chase Waggener
 * 11/23/17
 * 
 * This class provides functionality for a MQTT topic. It contains the topic
 * and current state, as well as a list of possible states and callbacks to
 * be run when the state changes.
 * 
 * 
 */


#ifndef MQTT_TOPIC_H
#define MQTT_TOPIC_H

#include "PubSubClient.h"
#include <ESP8266WiFi.h>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <functional>

typedef PubSubClient mqtt_client_t;
typedef std::string mqtt_topic_t;
typedef std::string mqtt_state_t;



/* For the client handler */
typedef PubSubClient mqtt_client_t;

/* Callback type */
typedef std::function<void()> mqtt_callback_t;

/* Callback list type */
typedef std::unordered_map<mqtt_state_t, mqtt_callback_t> mqtt_callback_list_t;

/**
 * Map each topic to a pair
 * Pair holds current state and a map of possible payloads and their corresponding
 * callbacks
 */
typedef std::unordered_map<
    mqtt_topic_t, 
    std::pair <mqtt_state_t, mqtt_callback_list_t>
> mqtt_container_t;

class MqttTopic {

public:


    /**
     * @brief Constructor that takes a MQTT topic name
     * 
     * @param topic The MQTT topic to which this object will be bound
     */
    MqttTopic(mqtt_topic_t topic);

    /**
     * @brief Destructor. Removes object from static topic list
     * 
     */
    ~MqttTopic();

    /**
     * @brief Sets the current MQTT state. This is probably called in the
     * MQTT client callback function. This function also runs appropriate
     * callbacks on state change
     * 
     * @param payload The new state to set for this topic
     * 
     * @return this
     */
    MqttTopic &state(const mqtt_state_t payload);
    const mqtt_state_t state() { return _state; }

    /**
     * @brief Publishes the given payload and sets the state for this topic.
     * This does not run callbacks associated with the new state
     * 
     * @param payload The new state to set for this topic
     * 
     * @return bool
     *   - true if successful
     *   - false otherwise
     */
    const bool publish(const mqtt_state_t payload);

    /**
     * @brief Adds a payload/callback pair to this topic
     * 
     * @param payload The payload that will trigger the callback
     * @param callback A pointer to the callback function
     *  
     * @return this
     */ 
    MqttTopic &callback(const mqtt_state_t payload, mqtt_callback_t callback);
    MqttTopic &callback(std::pair<mqtt_state_t, mqtt_callback_t> pair);

    /**
     * @brief Adds a callback to run on any incoming payload. Use this to handle
     * callbacks where the payload must be processed using more than basic matching
     * 
     * @note These callbacks are added using an empty payload key, aka std::string("")
     * 
     * @param callback A pointer to the callback function
     * 
     * @return this
     */
    MqttTopic &callback(mqtt_callback_t callback);

     /**
      * @brief Removes a payload / callback pair from this topic. You must pass
      * both payload and callback because the container is a multimap.
      * 
      * @param payload The payload of the callback to remove
      * @param callback The callback to remove
      * 
      */
    MqttTopic &remove(const mqtt_state_t payload, mqtt_callback_t * const callback);
    MqttTopic &remove(std::pair<mqtt_state_t, mqtt_callback_t*> pair);
    MqttTopic &remove(mqtt_callback_t * const callback);

    /**
     * @brief Returns the topic as a string
     * 
     */
    const std::string topic() { return _TOPIC; }

    // template <typename... Args>
    // const int publish(Args... args) {
    //     return sClient.publish(args...);
    // }


    // template <typename... Args>
    // static bool connect(Args... args) {
    //     return sCli
    // }

    template <typename... Args>
    static void setServer(Args... args) {
        _sClient.setServer(args...);
    }

    //static int state() { return _sClient.state(); }

    static PubSubClient *const clientPointer() { return &_sClient; }

    static void loop();

    /**
     * @brief Sets the callback to be run on disconnection from the MQTT
     * server
     * 
     * @param callback The callback to run
     * 
     */
    static void onDisconnect(mqtt_callback_t callback) { _onDisconnect = callback; }

    /**
     * @brief Sets the callback to be run on connection to the MQTT
     * server
     * 
     * @param callback The callback to run
     * 
     */
    static void onConnect(mqtt_callback_t callback) { _onConnect = callback; }



private:

    const mqtt_topic_t _TOPIC;
    mqtt_state_t _state;

    // Callbacks for client actions
    static mqtt_callback_t _onConnect, _onDisconnect;

    // Holds states and pointers to callback functions
    std::unordered_multimap<mqtt_state_t, mqtt_callback_t> _callbacks;

    /**
     * Holds a static list of all instantiated topics
     * This is needed to resubscribe all topics on reconnect
     */
    static std::unordered_map<mqtt_topic_t, MqttTopic*> _sTopics;

    static WiFiClient _sEspCli;
    static PubSubClient _sClient;
    static const char* _ID;

    static void _sHandleConnection();

    /**
     * @brief Resubscribes to the MQTT topic
     * 
     * @return
     *   - true if successful
     *   - false otherwise
     */
    const bool _resubscribe();

    /**
     * @brief Static function that serves as the highest level callback for MQTT
     * events. This callback will search all instantiated MqttDevice objects and
     * take the appropriate actions
     * 
     * @param topic The topic on which an event occurred
     * @param payload A byte array pointer containing the payload
     * @param length The length of the payload
     * 
     */
    static void _sCallback(char* topic, byte* payload, unsigned int length);

};

#endif