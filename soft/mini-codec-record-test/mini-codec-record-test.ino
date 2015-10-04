/* mini trash rec/play test; basically, the pjrc "recorder" example ; needs modded wm8731.h/cpp files (in "audio lib")
*/


#include <Bounce.h>
#include <Audio.h>
#include <i2c_t3.h>
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

#define BUT1 1
#define BUT2 4
#define LED1 2
#define LED2 5
#define CS_MEM 6
#define CS_SD 10

/*
// remaining i/o, unused here :
#define POT1 A10
#define POT2 A11
#define POT3 A7
#define POT4 A6
#define CV1  A1
#define CV2  A2
#define CLK1 0
#define CLK2 3
*/

Bounce buttonRecord = Bounce(BUT1, 50);
Bounce buttonPlay =   Bounce(BUT2, 50);

const int audio_Input = AUDIO_INPUT_LINEIN;

enum MODE {
  STOP,
  REC,
  PLAY  
};


int mode = 0;  // 0=stopped, 1=recording, 2=playing
int wait = 0;
uint32_t timestamp = 0;
uint32_t file_length = 0;
const uint16_t FADE = 100;

// The file where data is recorded
File frec;

void setup() {

  // buttons 1,2  
  pinMode(BUT1, INPUT_PULLUP);
  pinMode(BUT2, INPUT_PULLUP);
  pinMode(LED1, OUTPUT); 
  pinMode(LED2, OUTPUT); 
  
  pinMode(CS_MEM, OUTPUT); 
  digitalWrite(CS_MEM, HIGH);
 
  AudioMemory(60);

  wm8731.enable(); // this enables both the ADC and DAC
  wm8731.inputLevel(0.7f); // input level
  wm8731.volume(0.7f); // output level

  SPI.setMOSI(7);
  SPI.setSCK(14);
  if (!(SD.begin(CS_SD))) {

    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}


void loop() {

  buttonRecord.update();
  buttonPlay.update();

  // rec button = top button
  // press to record; press again to stop; when playing, pressing "rec" stops playing
  if (buttonRecord.fallingEdge()) {

    if (mode == STOP) { 
          startRecording();  
          digitalWriteFast(LED1, HIGH); 
          digitalWriteFast(LED2, LOW); 
    }
     else if (mode == PLAY) { 
          stopPlaying();    
          digitalWriteFast(LED1, LOW);  
          digitalWriteFast(LED2, LOW); 
    }
    else if (mode == REC) { 
          stopRecording();  
          digitalWriteFast(LED1, LOW);  
          digitalWriteFast(LED2, LOW); 
    }
  }
  // play button = lower button
  // press to play file; press again to stop; when recording, pressing "play" stops recording
  if (buttonPlay.fallingEdge()) {
  
    if (mode == STOP) { 
            startPlaying();       
            digitalWriteFast(LED1, LOW); 
            digitalWriteFast(LED2, HIGH); 
    }
    else if (mode == REC) { 
            stopRecording(); 
            digitalWriteFast(LED1, LOW); 
            digitalWriteFast(LED2, LOW); 
    }
    else if (mode == PLAY) { 
             stopPlaying();   
             digitalWriteFast(LED1, LOW); 
             digitalWriteFast(LED2, LOW); 
    }
  }

  if (mode == REC) continueRecording();

  if (mode == PLAY) {
    
    continuePlaying();
    // fade out file when we reach near the end:
    if (raw_file.positionMillis() > file_length - FADE) fade1.fadeOut(FADE);
  }
  
  if (wait && millis() - timestamp > FADE) {
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
    mode = REC;
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
  mode = STOP;
}

void startPlaying() {
  fade1.fadeIn(50);
  raw_file.play("RECORD.RAW");
  mode = PLAY;
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
      fade1.fadeOut(FADE);
      wait = 0x1;
      timestamp = millis();
      mode = STOP;
  }
}
