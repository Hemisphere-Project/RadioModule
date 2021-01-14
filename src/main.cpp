#include <Arduino.h>
#include "M5Atom.h"
#include <TEA5767.h>
TEA5767 radio = TEA5767();

short minlvl = 10; //Required Signal Level (0-5=unuseable, 6-9=poor, 10-12=good, 13-14=really good, 15=excellent)
bool muted = false;

void setup()
{
    M5.begin(true, false, true);
    delay(50);
    M5.dis.fillpix(0xf00000);

    Serial.begin(115200); //Initialize Serial Connection
    
    Wire.begin(21, 25);
    radio.init(&Wire, minlvl); //Initialize needed Features and search for Stations
    radio.setFrequency(103.0);
    Serial.println(radio.getSignalLevel());
}

uint8_t FSM = 0;

void loop()
{
    if (M5.Btn.wasPressed())
    {
      Serial.println(radio.getSignalLevel());
      muted = !muted;
      radio.setMuted(muted);
    }
    delay(50);
    M5.update();
}

void oz()
{
   
}
