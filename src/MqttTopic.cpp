#include "MqttTopic.h"

std::unordered_map<mqtt_topic_t, MqttTopic*> MqttTopic::_sTopics;
WiFiClient MqttTopic::_sEspCli;
PubSubClient MqttTopic::_sClient("dummyServer", 0, _sCallback, _sEspCli);
const char* MqttTopic::_ID = String(ESP.getChipId()).c_str();
mqtt_callback_t MqttTopic::_onConnect, MqttTopic::_onDisconnect;

/**
 * Constructor
 * 
 * Adds topic fron constructor to a static list of topics and
 * subscribes to the new topic if possible
 * 
 */
MqttTopic::MqttTopic(mqtt_topic_t topic)
:
_TOPIC(topic)
{
    // Add this topic to the static list of instantiated topics
    _sTopics.emplace(std::make_pair(topic, this));

    // // If connected, subscribe to the new topic
    if( _sClient.connected() )
        _sClient.subscribe(_TOPIC.c_str());
}

/**
 * Destructor
 * 
 * Unsubscribes from the given topic and removes this topic from
 * the static topic list
 * 
 */
MqttTopic::~MqttTopic() {
    // Unsubscribe
    _sClient.unsubscribe(_TOPIC.c_str());

    // Lookup and remove topic from static topic list
    auto i = _sTopics.find(_TOPIC);
    if( i != _sTopics.end() ) _sTopics.erase(i);
}

/**
 * First updates the member variable that tracks state to
 * match the incoming MQTT payload.
 * 
 * Next searches for callbacks assigned to the incoming payload
 * 
 * Finally searches for callbacks assigned to run for any payload
 */
MqttTopic &MqttTopic::state(const mqtt_state_t PAYLOAD) {
    _state = PAYLOAD;

    if( _callbacks.empty() ) return *this;

    // Find all callbacks for the given state
    auto range = _callbacks.equal_range(PAYLOAD);
    for(auto i = range.first; i != range.second; i++) {
        (i->second)();
    }

    range = _callbacks.equal_range("");
    for(auto i = range.first; i != range.second; i++) {
        (i->second)();
    }

    return *this; 
}


/**
 * Publishes a payload by forwarding to the MQTT client class
 * 
 */
const bool MqttTopic::publish(const mqtt_state_t PAYLOAD) {
    return _sClient.publish(_TOPIC.c_str(), PAYLOAD.c_str());
} 

/**
 * Assigns a callback function to run on a given incoming payload.
 * Returns a handle to the callback so it can be removed later
 * 
 */
mqtt_callback_ptr_t MqttTopic::callback(const mqtt_state_t payload, mqtt_callback_t fun) {
    auto ret = _callbacks.emplace( std::make_pair(payload, fun) );
    return ret;
} 

/**
 *  Assigns a callback to run on any payload for this topic
 * 
 */
mqtt_callback_ptr_t MqttTopic::callback(std::function<void(mqtt_state_t)> fun) {
    return _callbacks.emplace( 
        std::make_pair("", [this, fun](){
            fun(this->state());
        })
    );
} 

/**
 * First check if we are connected.
 * 
 * If not connected and we were connected earlier, we
 * need to run the onDisconnect callback.
 * 
 * If disconnected try to reconnect. If that works, 
 * resubscribe to all MQTT topics
 * 
 * Run loop() for MQTT client
 * 
 */
void MqttTopic::loop() {
    static bool previous_state = false;

    // Maintain connection
    if( !_sClient.connected() ) {

        // Run disconnect callback if needed
        if(previous_state) {
            previous_state = !previous_state;
            if(_onDisconnect) _onDisconnect();
        }

        // If we reconnect, resubscribe to all topics
        if( _sClient.connect(_ID) ) {
            previous_state = true;
            for( auto i : _sTopics ) {
                i.second->_resubscribe();
            }
            if(_onConnect) _onConnect();
            return;
        }
    }
    _sClient.loop();
}

const bool MqttTopic::_resubscribe() {
    return _sClient.subscribe( _TOPIC.c_str() );
}

/**
 * Matches PubSubClient callback handler
 * 
 * See if there exists a MqttTopic object that matches the incoming message
 * If there is a match, update the state for that topic
 */
void MqttTopic::_sCallback(char* topic, byte* payload, unsigned int length) {
    const auto i = _sTopics.find(std::string(topic));
    if( i == _sTopics.end() ) return;

    i->second->state(std::string(reinterpret_cast<const char*>(payload), length));
}
