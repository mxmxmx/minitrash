// test for spi ram . based on paul stoffregen's example -- 
// https://forum.pjrc.com/threads/29276-Limits-of-delay-effect-in-audio-library


#include <Audio.h>
#include <Wire.h> 
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioInputI2S              i2s_ADC;                   
AudioEffectEnvelope        env;
AudioEffectDelayExternal   dly1(AUDIO_MEMORY_23LC1024, 570);
AudioEffectDelayExternal   dly2(AUDIO_MEMORY_23LC1024, 570);
AudioMixer4                mix;
AudioOutputI2S             i2s_DAC;

AudioConnection            patchCord1(i2s_ADC, env);
AudioConnection            patchCord2(env, dly1);
AudioConnection            patchCord3(env, dly2);
AudioConnection            patchCord4(dly1, 0, mix, 0);
AudioConnection            patchCord5(dly2, 0, mix, 1);
AudioConnection            patchCord6(env, 0, mix, 2);
AudioConnection            patchCord7(mix, 0, i2s_DAC, 0);
AudioConnection            patchCord8(mix, 0, i2s_DAC, 1);

AudioControlWM8731         wm8731;  

const int audio_Input = AUDIO_INPUT_LINEIN;

#define CS_MEM 6
#define CS_SD 10

#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

#define POT1 A10
#define POT2 A11

// remaining i/o, unused here: 
/*
#define POT3 A7
#define POT4 A6

#define CV1  A1
#define CV2  A2

#define CLK1 0
#define CLK2 3

#define SW1 1
#define SW2 4

#define LED1 2
#define LED2 5
*/

IntervalTimer ADC_timer;
volatile uint16_t _ADC;

void ADC_Callback() 
{
  _ADC = true;
}

void setup() {
  
        AudioMemory(10);
        
        wm8731.enable();
        wm8731.inputSelect(audio_Input); // not doing anything
        wm8731.inputLevel(0.9f); // 0.0 - 1.0
        wm8731.volume(0.7f);

        SPI.setMOSI(SDCARD_MOSI_PIN);
        SPI.setSCK(SDCARD_SCK_PIN);
        
        pinMode(CS_MEM, OUTPUT); 
        digitalWrite(CS_MEM, HIGH);
         
        pinMode(CS_SD, OUTPUT); 
        digitalWrite(CS_SD, HIGH);
        
        ADC_timer.begin(ADC_Callback, 5000);
}

void loop() {
  
        if (_ADC) {
            _ADC = false;
            uint16_t p1 = analogRead(POT1);
            uint16_t p2 = analogRead(POT2);
            dly1.delay(0, p1+2);
            dly2.delay(0, p2+2);
        }
        env.noteOn();
        delay(50);
        env.noteOff();
        delay(1000);

}
