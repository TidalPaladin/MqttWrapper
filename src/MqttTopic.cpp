#include "MqttTopic.h"


std::unordered_map<mqtt_topic_t, MqttTopic*> MqttTopic::_sTopics;
WiFiClient MqttTopic::_sEspCli;
PubSubClient MqttTopic::_sClient("dummyServer", 0, _sCallback, _sEspCli);
const char* MqttTopic::_ID = String(ESP.getChipId()).c_str();

mqtt_callback_t MqttTopic::_onConnect, MqttTopic::_onDisconnect;

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

MqttTopic::~MqttTopic() {
    // Unsubscribe
    _sClient.unsubscribe(_TOPIC.c_str());

    // Lookup and remove topic from static topic list
    auto i = _sTopics.find(_TOPIC);
    if( i != _sTopics.end() ) _sTopics.erase(i);
}


MqttTopic &MqttTopic::state(const mqtt_state_t PAYLOAD) {

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

const bool MqttTopic::publish(const mqtt_state_t PAYLOAD) {
    _state = PAYLOAD;
    return _sClient.publish(_TOPIC.c_str(), PAYLOAD.c_str());
}

  
MqttTopic &MqttTopic::callback(const mqtt_state_t payload, mqtt_callback_t callback) {
    _callbacks.emplace( std::make_pair(payload, callback) );
    return *this;
} 
MqttTopic &MqttTopic::callback(mqtt_callback_t callback) {
    _callbacks.emplace( std::make_pair("", callback) );
    return *this;
} 
// MqttTopic &MqttTopic::callback(std::pair<mqtt_state_t, mqtt_callback_t> pair) {
//     _callbacks.insert(pair); 
//     return *this;
// } 




// MqttTopic &MqttTopic::remove(mqtt_callback_t * const callback) {
//     remove(std::make_pair("",callback));
// }
// MqttTopic &MqttTopic::remove(const mqtt_state_t payload, mqtt_callback_t * const callback) {
//     remove(std::make_pair(payload,callback));
// }
// MqttTopic &MqttTopic::remove(std::pair<mqtt_state_t, mqtt_callback_t*> pair) {
//     // Find all callbacks for the given state
//     auto range = _callbacks.equal_range(pair.first);

//     // Use first and second iterators to check and remove matching pairs
//     for(auto i = range.first; i != range.second; i++) {
//         if(i->second == pair.second) {
//             _callbacks.erase(i);
//         }
//     }
 
//     return *this;
// }

void MqttTopic::loop() {
    static bool previous_state = false;
    if( !_sClient.connected() ) {

        if(previous_state) {
            previous_state = !previous_state;
            if(_onDisconnect) _onDisconnect();
        }

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

void MqttTopic::_sHandleConnection() {
    
}

const bool MqttTopic::_resubscribe() {
    return _sClient.subscribe( _TOPIC.c_str() );
}

void MqttTopic::_sCallback(char* topic, byte* payload, unsigned int length) {
    const auto i = _sTopics.find(std::string(topic));
    if( i == _sTopics.end() ) return;

    i->second->state(std::string(reinterpret_cast<const char*>(payload), length));
}
