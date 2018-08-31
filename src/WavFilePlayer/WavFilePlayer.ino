// WAV file player for Gaston
// based on simple wav player demo by PJRC, see:
//   https://www.pjrc.com/teensy/td_libs_Audio.html
//
// This example code is in the public domain.

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioPlaySdWav           playWav1;
AudioOutputI2S           audioOutput;
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

int monitorOutput=false;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  AudioMemory(8);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.2);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

void playFile(const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);

  playWav1.play(filename);
  // A brief delay for the library read WAV info
  delay(5);
}

char filename[] = "x.wav";

void loop() {
  int incomingByte;

  float vol = analogRead(15);
  vol = vol / 3000;
  sgtl5000_1.volume(vol);

  if (Serial1.available() > 0) {
    incomingByte = Serial1.read();

    switch (incomingByte) {
       case '?':  monitorOutput=true; break;
       case '!':  monitorOutput=false; break;
       default: 
            if (monitorOutput) { Serial.write(incomingByte); break;}
            else {
              if (incomingByte=='0') { playWav1.stop(); break; }
              filename[0]=incomingByte;
              if (playWav1.isPlaying()) {
                 playWav1.stop();
                 delay(50);
              }
              playWav1.play(filename);
            }
    }
  }
  
}

