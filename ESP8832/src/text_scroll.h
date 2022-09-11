#pragma once

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

#include <string>

constexpr const char* TEXT_SPACER = "  ";

class TextScroll
{
    public:
        TextScroll(Adafruit_SSD1306& ssd) : ssd(ssd), timeNextStep(0), msStep(30), baseText(""), finalText(""), size(1), currentX(0)
        {}

        void speed(unsigned ms){
            msStep = ms;
        }

        void setText(std::string text){
            if( baseText == text){ return; }

            baseText = text;
            currentX = 0;

            generateFinalString();
        }

        void setTextSize(uint8_t size){
            this->size = size;
            generateFinalString();
        }

        void scroll(){
            if( millis() < timeNextStep ){ return; }
            
            if( currentX <= lastPositionX ){ currentX = 0; }

            ssd.setTextSize(size);
            ssd.setCursor(currentX, ssd.getCursorY());
            ssd.clearDisplay();
            ssd.print(finalText.c_str());
            ssd.display();

            currentX--;
            timeNextStep = millis() + msStep;
        }

        void reset(){
            currentX = 0;
        }

    private:
        Adafruit_SSD1306 &ssd;
        unsigned long timeNextStep;
        unsigned msStep;
        std::string baseText;
        std::string finalText;
        uint8_t size;
        int16_t currentX;
        int16_t lastPositionX;

        void generateFinalString(){
            int16_t n;
            uint16_t w;

            ssd.setTextSize(size);
            ssd.getTextBounds(baseText.c_str(), 0, 0, &n, &n, &w, (uint16_t*)&n);

            if( w <= ssd.width() ){
                finalText = baseText;
                lastPositionX = 0;
                return;
            }
            
            uint8_t overflow = (ssd.width() - w) / 8;
            ssd.getTextBounds((baseText + TEXT_SPACER).c_str(), 0, 0, &n, &n, &w, (uint16_t*)&n);

            finalText = baseText + TEXT_SPACER + baseText.substr(0, overflow - 1);
            lastPositionX = -1 * (w);
        }
};


