#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class RingRunning{
    public:
        RingRunning(Adafruit_NeoPixel ring) : 
            ring(ring), runningPixels(1), color(ring.Color(255, 255, 255)),
            step(0), msStep(50), timeNextStep(0)
        {
        }

        void init(){
            ring.begin();
            allOff();
        }

        void timePerStep(unsigned ms){
            msStep = ms;
        }

        void setColor(uint32_t color){
            this->color = color;
        }

        void setColor(uint8_t r, uint8_t g, uint8_t b){
            setColor(Adafruit_NeoPixel::Color(r, g, b));
        }

        void nbRunningPixels(uint8_t n){
            if(n >= ring.numPixels() || n == 0 ){ return; }
            runningPixels = n;
        }

        void nextStep(){
            if( millis() < timeNextStep ){ return; }

            ring.fill();
            
            for(uint8_t i = 0; i < runningPixels; i++)
                ring.setPixelColor(getCircularIndex( step+i ), color);
            
            ring.show();

            step = getCircularIndex(step+1); //(step+1) % ring.numPixels();
            timeNextStep = millis() + msStep;
        }

        void allOn(){
            ring.fill(color);
            ring.show();
            step = 0;
        }

        void allOn(uint32_t c){
            ring.fill(c);
            ring.show();
            step = 0;
        }

        void allOff(){
            ring.fill();
            ring.show();
            step = 0;
        }

    private:
        Adafruit_NeoPixel ring;
        uint8_t runningPixels;
        uint32_t color;
        int8_t step;
        unsigned msStep;
        unsigned long timeNextStep;

        uint8_t getCircularIndex( int8_t s ){
            if( s < 0 ){
                return ring.numPixels() - s;
            }
            else if( s >= ring.numPixels() ){
                return s - ring.numPixels();
            }
            else{
                return s;
            }
        }
        
};