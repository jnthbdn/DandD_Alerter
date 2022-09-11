#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>
#include <deque>

#include "eeprom_settings.h"
#include "settingServer.h"
#include "ring_running.h"
#include "mqttAlerter.h"
#include "text_scroll.h"

#include "utils.h"

constexpr uint8_t PIN_BUTTON = D4;
constexpr uint8_t NB_RING_LED = 12;
constexpr uint8_t PIN_RING_LED = D3;
// constexpr const char* MQTT_SERVER = "192.168.1.16";
constexpr const char* MQTT_SERVER = "broker.hivemq.com";
constexpr uint16_t MQTT_PORT = 1883;
constexpr uint8_t MAX_NAME_LIST_SIZE = 5;

EEPROM_Settings settings;
SettingServer settingServer(settings);
MqttAlerter mqtt;

bool isAP = false;
String deviceID;

Adafruit_SSD1306 ssd(128, 32);
TextScroll textScroll(ssd);

Adafruit_NeoPixel ring(NB_RING_LED, PIN_RING_LED, NEO_GRB + NEO_KHZ800);
RingRunning runningRing(ring);
std::deque<std::string> waitingNames;

void ap_loop(){
  settingServer.handleClient();
  runningRing.nextStep();
}

void main_loop(){
  mqtt.handleNewMessage();

  while(mqtt.hasNewName() > 0){
    std::string tmp = mqtt.getReceiveName();
    if( waitingNames.size() < MAX_NAME_LIST_SIZE ){
      waitingNames.push_back( tmp );
    }
  }

  if( waitingNames.size() > 0 ){

    if(digitalRead(PIN_BUTTON) == LOW){
      waitingNames.pop_front();
      textScroll.reset();

      while(digitalRead(PIN_BUTTON) == LOW){
        yield();
      }
      return;
    }

    ssd.setCursor(0, 4);
    textScroll.setText(waitingNames[0]);
    textScroll.scroll();
    runningRing.nextStep();
  }
  else{
    ssd.setTextSize(1);
    ssd.clearDisplay();
    ssd.setCursor(0, 0);
    ssd.print("D&D Alerter ID:");
    ssd.setTextSize(3);
    ssd.setCursor(15, 10);
    ssd.print(deviceID);
    ssd.display();
    
    runningRing.allOff();
  }
}

void onMqttReconnect(){
  ssd.clearDisplay();
  ssd.setTextSize(1);
  ssd.setCursor(0, 0);
  ssd.print("MQTT Connection...");
  ssd.display();
  runningRing.allOn(ring.Color(255, 128, 0));
}

void onMqttConnectionError(int){
  ssd.setCursor(0, 8);
  ssd.print("MQTT FAILED !");
  ssd.setCursor(0, 24);
  ssd.print("Retry in 5 sec.");
  ssd.display();
  runningRing.allOn(ring.Color(255, 0, 0));

  delay(5000);
}

void onMqttConnectionSuccess(){
  ssd.setCursor(0, 8);
  ssd.print("MQTT Success !");
  ssd.display();

  runningRing.allOn(ring.Color(0, 255, 0));
  delay(1000);
  runningRing.allOff();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  
  ssd.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  ssd.clearDisplay();
  ssd.setTextWrap(false);
  ssd.setTextSize(1);
  ssd.setTextColor(SSD1306_WHITE);
  ssd.setCursor(0,0);
  ssd.print("Starting...");
  ssd.display();

  textScroll.speed(10);

  deviceID = generateAlphaNum(5);

  runningRing.init();
  runningRing.nbRunningPixels(3);
  runningRing.setColor(255, 255, 255);
  runningRing.allOn();
  
  settings.init();

  mqtt.init(deviceID);
  mqtt.setOnReconnect(onMqttReconnect);
  mqtt.setOnConnectionError(onMqttConnectionError);
  mqtt.setOnConnectionSuccess(onMqttConnectionSuccess);

  Wire.setClock(400000);

  ssd.setCursor(0,8);
  ssd.print("WiFi connection...");
  ssd.display();

  if( digitalRead(PIN_BUTTON) == HIGH && tryConnectWiFi(settings.getSSID().c_str(), settings.getPassword().c_str()) ){
    isAP = false;
    runningRing.allOn( Adafruit_NeoPixel::Color(0, 255, 0));
    mqtt.start(MQTT_SERVER, MQTT_PORT);

    ssd.setCursor(0,16);
    ssd.print("All right !");
    ssd.setCursor(0,2);
    ssd.display();
    textScroll.setTextSize(4);
    
    runningRing.setColor(255, 0, 0);
    runningRing.timePerStep(20);

    delay(1500);
  }
  else{
    isAP = true;

    settingServer.startServer();
    runningRing.setColor(0, 0, 255);

    ssd.setCursor(0,16);
    ssd.print("Connection failed !");
    ssd.display();

    delay(1500);

    ssd.setTextSize(1);
    ssd.clearDisplay();
    ssd.setCursor(0,0);
    ssd.print("WiFi not configured !");
    ssd.setCursor(0,8);
    ssd.print("Connect to network:");
    ssd.setCursor(0,16);
    ssd.print("   DD-Alert_SETUP");
    ssd.setCursor(0,24);
    ssd.print("then http://8.8.8.8/");
    ssd.display();
  }
}

void loop() {
  if( isAP ){
    ap_loop();
  }
  else{
    main_loop();
  }
}
