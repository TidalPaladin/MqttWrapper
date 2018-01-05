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
#include <sstream>

typedef PubSubClient mqtt_client_t;
typedef std::string mqtt_topic_t;
typedef std::string mqtt_state_t;

/* For the client handler */
typedef PubSubClient mqtt_client_t;

/* Callback type */
typedef std::function<void()> mqtt_callback_t;

/* Callback list type */
typedef std::unordered_map<mqtt_state_t, mqtt_callback_t> mqtt_callback_list_t;

/* Handle for a created callback so it can be removed later */
typedef mqtt_callback_list_t::const_iterator mqtt_callback_ptr_t;  

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
     * TODO should this be a private member?
     * 
     * @note The difference between state() and publish() is that state()
     * handles callbacks for an incoming payload while publish() simply
     * publishes any message on the given topic
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
     * @brief Handles publishing for non string data types
     * 
     * @param payload The payload to be published
     * 
     * @return bool
     *   - true if successful
     *   - false otherwise
     */
    template <typename T>
    const bool publish(const T payload) {
        // std::ostringstream isnt available so we have to do this
        return publish(
            std::string(String(payload).c_str())
        );
    }

    /**
     * @brief Adds a payload/callback pair to this topic
     * 
     * @param payload The payload that will trigger the callback
     * @param callback A callback std::function
     *  
     * @return A handle that can be used to remove the callback
     */ 
    mqtt_callback_ptr_t callback(const mqtt_state_t payload, mqtt_callback_t callback);
    mqtt_callback_ptr_t callback(std::pair<mqtt_state_t, mqtt_callback_t> pair);

    /**
     * @brief Adds a callback to run on any incoming payload. Use this to handle
     * callbacks where the payload must be processed using more than basic matching.
     * The callback should accept a mqtt_state_t parameter which will receive the incoming
     * payload
     * 
     * @note These callbacks are added using an empty payload key, aka std::string("")
     * 
     * @param callback A callback std::function with parameter mqtt_state_t
     * 
     * @return A handle that can be used to remove the callback
     */
    mqtt_callback_ptr_t callback(std::function<void(mqtt_state_t)> callback);

     /**
      * @brief Removes a callback using the pointer generated when it was added
      *
      * Not currently working - iterators will be invalidated by rehashing
      * 
      * @param payload The payload of the callback to remove
      * @param callback The callback to remove
      * 
      */
    // MqttTopic &remove(mqtt_callback_ptr_t callback);

    /**
     * @brief Returns the topic as a string
     * 
     * @return The MQTT topic for this object
     */
    const std::string topic() const { return _TOPIC; }


    /**
     * @brief Assigns a MQTT server location
     * 
     * @param Perfectly forwarded to PubSubClient
     * 
     */
    template <typename... Args>
    static void setServer(Args... args) {
        _sClient.setServer(args...);
    }

    /**
     * @brief Fetches the state of the underlying MQTT client
     * (PubSubClient)
     * 
     * @return The status code of the MQTT client
     */
    static int clientState() { return _sClient.state(); }

    /**
     * @brief Call this in loop() to maintain MQTT connection
     * and handle incomding messages
     * 
     */
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


};


#endif