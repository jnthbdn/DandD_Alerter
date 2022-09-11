#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <vector>



const char * StatusToString(wl_status_t status){
  switch (status)
  {

    case WL_IDLE_STATUS    :
      return "WL_IDLE_STATUS    ";
    
    case WL_NO_SSID_AVAIL  :
      return "WL_NO_SSID_AVAIL  ";
    
    case WL_SCAN_COMPLETED :
      return "WL_SCAN_COMPLETED ";
    
    case WL_CONNECTED      :
      return "WL_CONNECTED      ";
    
    case WL_CONNECT_FAILED :
      return "WL_CONNECT_FAILED ";
    
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    
    case WL_WRONG_PASSWORD :
      return "WL_WRONG_PASSWORD ";

    case WL_DISCONNECTED   :
      return "WL_DISCONNECTED   ";
    
    default:
      return "UNKNOWN";
  }
}

bool tryConnectWiFi(const char* ssid, const char* pass, unsigned long timeout = 10000){
    WiFi.mode(WiFiMode::WIFI_STA);
    WiFi.begin(ssid, pass);

    unsigned long start = millis();

    while( WiFi.status() != WL_CONNECTED && (millis() - start) < timeout){
        delay(250);
    }

    Serial.println(StatusToString(WiFi.status()));

    return (WiFi.status() == WL_CONNECTED);
}


constexpr const char* ALPHA_NUM = "0123456789ABCDEF";

String generateAlphaNum(size_t n){
    String result = "";

    randomSeed(ESP.getCycleCount() + analogRead(A0));
    
    for(size_t i = 0; i < n; ++i){
        size_t v = (size_t)random(0, 16);
        result += ALPHA_NUM[v];
    }

    return result;
}