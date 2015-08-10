/* mini trash rec/play test; basically, the pjrc "recorder" example ; needs modded control_wm8731.h/cpp files

*/


#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

AudioInputI2S            i2s_ADC;                   
AudioRecordQueue         queue1;         
AudioPlaySdRaw           raw_file;    
AudioOutputI2S           i2s_DAC;           
AudioConnection          patchCord1(i2s_ADC,  0, queue1, 0);
AudioConnection          patchCord3(raw_file, 0, i2s_DAC, 0);
AudioConnection          patchCord4(raw_file, 0, i2s_DAC, 1);
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

#define CS_MEM 6

Bounce buttonRecord = Bounce(BUT1, 500);
Bounce buttonPlay =   Bounce(BUT2, 500);

const int audio_Input = AUDIO_INPUT_LINEIN;

int mode = 0;  // 0=stopped, 1=recording, 2=playing

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

  wm8731.enable();
  wm8731.inputSelect(audio_Input); // not doing anything
  wm8731.inputLevel(0.9f); // 0.0 - 1.0
  wm8731.volume(0.7f);

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

  if (buttonRecord.fallingEdge()) {
    // rec button pressed
    //Serial.print(mode);
    //Serial.print(" -- > ");
    if (mode == 0) { 
          startRecording();  
          digitalWriteFast(LED1, HIGH); 
          digitalWriteFast(LED2, LOW); 
    }
    else if (mode == 2) { 
          stopPlaying();    
          digitalWriteFast(LED1, LOW);  
          digitalWriteFast(LED2, LOW); 
    }
    else if (mode == 1) { 
          stopRecording();  
          digitalWriteFast(LED1, LOW);  
          digitalWriteFast(LED2, LOW); 
    }
  //Serial.println(mode);
  }

  if (buttonPlay.fallingEdge()) {
    // play button pressed
    //Serial.print(mode);
    //Serial.print(" -- > ");
    if (mode == 0) { 
            startPlaying();       
            digitalWriteFast(LED1, LOW); 
            digitalWriteFast(LED2, HIGH); 
    }
    else if (mode == 1) { 
            stopRecording(); 
            digitalWriteFast(LED1, LOW); 
            digitalWriteFast(LED2, LOW); 
    }
    else if (mode == 2) { 
             stopPlaying();   
             digitalWriteFast(LED1, LOW); 
             digitalWriteFast(LED2, LOW); 
    }
    //Serial.println(mode);
  }

  if (mode == 1) {
    continueRecording();
  }
  if (mode == 2) {
    continuePlaying();
  }
}


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
    mode = 1;
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
    elapsedMicros usec = 0;
    frec.write(buffer, 512);
  }
}

void stopRecording() {
  Serial.println("stopRecording");
  queue1.end();
  if (mode == 1) {
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    frec.close();
  }
  mode = 0;
}


void startPlaying() {
  Serial.println("startPlaying");
  raw_file.play("RECORD.RAW");
  mode = 2;
}

void continuePlaying() {
  if (!raw_file.isPlaying()) {
    raw_file.stop();
    mode = 0;
    digitalWriteFast(LED2, LOW);
  }
}

void stopPlaying() {
  Serial.println("stopPlaying");
  if (mode == 2) raw_file.stop();
  mode = 0;
}
