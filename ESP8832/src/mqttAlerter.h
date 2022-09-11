#include <Arduino.h>

#include <queue>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

constexpr size_t MAX_NAME_LENGTH = 20;

class MqttAlerter{

    public:
        MqttAlerter() : client(wifiClient), onReconnect(nullptr), onConnectionError(nullptr), onConnectionSuccess(nullptr)
        {
            randomSeed(ESP.getCycleCount() + analogRead(A0));
        }

        void init(String alerterID){
            this->alerterID = alerterID;
        }

        void start(const char* server, uint16_t port = 1883){
            client.setServer(server, port);
            client.setCallback( std::bind(&MqttAlerter::onMQTTReceive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) );
        }

        void handleNewMessage(){
            if(!client.connected()){
                reconnect();
            }

            client.loop();
        }

        uint8_t hasNewName(){
            return names.size();
        }

        std::string getReceiveName(){
            if( names.empty() ){ return ""; }

            std::string r = names.front();
            names.pop();

            return r;
        }

        void setOnReconnect(std::function<void()> fn){ onReconnect = fn; }
        void setOnConnectionError(std::function<void(int)> fn){ onConnectionError = fn; }
        void setOnConnectionSuccess(std::function<void()> fn){ onConnectionSuccess = fn; }


    private:
        String alerterID;
        WiFiClient wifiClient;
        PubSubClient client;
        std::queue<std::string> names;
        StaticJsonDocument<256> jsonDoc;

        std::function<void()> onReconnect;
        std::function<void(int)> onConnectionError;
        std::function<void()> onConnectionSuccess;

        void reconnect(){
            if( onReconnect != nullptr) onReconnect();
            while(!client.connected()){
                yield();
                String clientID = "DD_Alerter_" + alerterID + "_" + String(random(0xFFFF), HEX);
            
                if( !client.connect(clientID.c_str()) ){
                    if( onConnectionError != nullptr) onConnectionError(client.state());
                    delay(5000);
                }
            }
            client.subscribe(String( "DD_ALERTER_" + alerterID ).c_str());
            client.publish(String( "DD_ALERTER_" + alerterID ).c_str(), "Alerter ready !");
            if( onConnectionSuccess != nullptr) onConnectionSuccess();
        }

        void onMQTTReceive(char* topic, uint8_t* payload, unsigned int length){
            if( deserializeJson(jsonDoc, payload) != DeserializationError::Ok || !jsonDoc.containsKey("name") ){
                return;
            }

            names.push(jsonDoc["name"].as<std::string>().substr(0, MAX_NAME_LENGTH));
        }
};