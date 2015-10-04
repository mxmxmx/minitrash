/* mini trash rec/play test; basically, the pjrc "recorder" example ; needs modded wm8731 files (wm8731.h/cpp in audio lib)
*
*  top button / trig input = arm track + record, lower button / trig input = play
*  pots/CV unused
*  recording is mono (only left input is used); output is to L and R channel
*/

#include <Bounce.h>
#include <Audio.h>
#include <i2c_t3.h> // #include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Audio API: 
AudioInputI2S            i2s_ADC;                   
AudioRecordQueue         queue1;         
AudioPlaySdRaw           raw_file; 
AudioEffectFade          fade1;
AudioOutputI2S           i2s_DAC;           
AudioConnection          ac0(i2s_ADC,  0, queue1, 0);
AudioConnection          ac1(raw_file,  0, fade1, 0);
AudioConnection          ac2(fade1, 0, i2s_DAC, 0);
AudioConnection          ac3(fade1, 0, i2s_DAC, 1);
AudioControlWM8731       wm8731;     

// buttons :
#define BUT1 1
#define BUT2 4
#define LED1 2
#define LED2 5
// CS pins :
#define CS_MEM 6
#define CS_SD 10
// trig inputs :
#define CLK1 0
#define CLK2 3

// remaining i/o, unused here :
/*
#define POT1 A10
#define POT2 A11
#define POT3 A7
#define POT4 A6
#define CV1  A1
#define CV2  A2
*/

Bounce buttonRecord = Bounce(BUT1, 50);
Bounce buttonPlay =   Bounce(BUT2, 50);

// trigger inputs : 
volatile uint16_t _CLK1, _CLK2;

void CLK_ISR_1() { _CLK1 = true; }
void CLK_ISR_2() { _CLK2 = true; }

enum MODE {
  STOP,
  REC,
  PLAY, 
  ARM  
};

int mode = 0;  // 0=stopped, 1=recording, 2=playing, 3=armed
int wait = 0;
uint32_t timestamp = 0;
uint32_t file_length = 0;
const uint16_t FADE_OUT = 100;
const uint16_t FADE_IN = 100;

// The file where data is recorded :
File frec;

// blink top led when track is recorded :
IntervalTimer blink;
volatile uint16_t _LED = false;
const uint32_t BLINK_RATE = 100000;

void _blink() {
   
    if (mode == REC) { 
        digitalWriteFast(LED1, _LED);
       _LED = ~_LED & 1u;
    }
}

void setup() {

  // buttons 1,2  
  pinMode(BUT1, INPUT_PULLUP);
  pinMode(BUT2, INPUT_PULLUP);
  pinMode(LED1, OUTPUT); 
  pinMode(LED2, OUTPUT); 
  // 
  pinMode(CLK1, INPUT_PULLUP); 
  pinMode(CLK2, INPUT_PULLUP); 
  pinMode(CS_MEM, OUTPUT); 
  digitalWrite(CS_MEM, HIGH);
 
  AudioMemory(150);

  wm8731.enable(); // this enables both the ADC and DAC
  wm8731.inputLevel(0.7f); // input level
  wm8731.volume(0.7f); // output level

  SPI.setMOSI(7);
  SPI.setSCK(14);
  if (!(SD.begin(CS_SD))) {

    uint8_t xxx = 0x0;
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
      digitalWriteFast(LED1, xxx); 
      digitalWriteFast(LED2, xxx);
      xxx = ~xxx & 1u; 
    }
  }
  // trigger inputs 
  attachInterrupt(CLK1, CLK_ISR_1, FALLING);
  attachInterrupt(CLK2, CLK_ISR_2, FALLING);
  blink.begin(_blink, BLINK_RATE);
}


void loop() {

  buttonRecord.update();
  buttonPlay.update();

  // rec button = top button
  // press to arm track; press again to record; press again to stop; when playing, pressing "rec" stops playing and arms track
  if (_CLK1 || buttonRecord.fallingEdge()) {
    
    switch (mode) {
      
      case STOP: { 
        
        if (!_CLK1) { 
            // only arm track manually
            mode = ARM;
            digitalWriteFast(LED1, HIGH);
            digitalWriteFast(LED2, LOW);
        }
        break;
      }
      case REC: {
        
         stopRecording();
         digitalWriteFast(LED1, LOW);
         digitalWriteFast(LED2, LOW); 
         mode = STOP;
         break;
      }
      case PLAY: {
        
         if (!_CLK1) {
           // only arm track manually
           stopPlaying();
           digitalWriteFast(LED1, HIGH);
           digitalWriteFast(LED2, LOW); 
           mode = ARM;
         }
         break;
      } 
      case ARM: {
        
         startRecording();  
         digitalWriteFast(LED1, HIGH); 
         digitalWriteFast(LED2, LOW);
         mode = REC;
         break;    
      }
      default: break;
      
    }
    _CLK1 = false;
  }
  // play button = lower button
  // press to play file; press again to stop; when recording, pressing "play" does nothing 
  if (_CLK2 || buttonPlay.fallingEdge()) {
        
    switch (mode) {
      
      case STOP: {
        
        startPlaying();       
        digitalWriteFast(LED1, LOW); 
        digitalWriteFast(LED2, HIGH); 
        mode = PLAY;
        break;
        
       }
      
      case REC: {
        
        if (!_CLK2) {
          // only stop recording manually
          stopRecording();
          digitalWriteFast(LED1, LOW);
          digitalWriteFast(LED2, LOW); 
          mode = STOP;
        }
        else mode = REC;
        break;
      }
      
      case PLAY: {
        
         stopPlaying();   
         digitalWriteFast(LED1, LOW); 
         digitalWriteFast(LED2, LOW); 
         break;
      }
      
      case ARM: {
        
        if (!_CLK2) {
          // de-active manually
          mode = STOP;
        }
        break; 
      }
      
      default: break;
    }
    _CLK2 = false;
  }

  if (mode == REC) continueRecording();

  if (mode == PLAY) {
        continuePlaying();
        // fade out file when we reach near the end :
        if (raw_file.positionMillis() > file_length - FADE_OUT) fade1.fadeOut(FADE_OUT);
    }
  
  if (wait && millis() - timestamp > FADE_OUT) {
     wait = 0x0;
     raw_file.stop();
   }
}


// misc functions: 

void startRecording() {
  //  start rec
  if (SD.exists("RECORD.RAW")) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove("RECORD.RAW");
  }
  frec = SD.open("RECORD.RAW", FILE_WRITE);
  if (frec) {
    queue1.begin();
  }
}

void continueRecording() {

  if (queue1.available() >= 2) {
    byte buffer[512];
    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer+256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    // write all 512 bytes to the SD card
    //elapsedMicros usec = 0;
    frec.write(buffer, 512);
  }
}

void stopRecording() {
  queue1.end();
  if (mode == 1) {
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    frec.close();
  }
}

void startPlaying() {
  
  fade1.fadeIn(FADE_IN);
  raw_file.play("RECORD.RAW");
  file_length = raw_file.lengthMillis();
}

void continuePlaying() {
  if (!raw_file.isPlaying()) {
    raw_file.stop();
    mode = STOP;
    digitalWriteFast(LED2, LOW);
  }
}

void stopPlaying() {
  
  if (mode == PLAY) {
      fade1.fadeOut(FADE_OUT);
      wait = 0x1;
      timestamp = millis();
      mode = STOP;
  }
}
