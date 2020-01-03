/**
 * JoniBOX, based on TonUINO
 * @see https://github.com/xfjx/TonUINO
 * @see https://www.voss.earth/tonuino/
 */

#include <DFMiniMp3.h>
#include <EEPROM.h>
#include <JC_Button.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include "Config.h"
#include "Variables.h"
#include "Functions.h"

// Create MFRC522 Card Reader Instance
MFRC522 mfrc522(PIN_CR_SS, PIN_CR_RST); 

// Arduino nano has only one hardware serial, DFPlayer is connected via software serial
SoftwareSerial mySoftwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Create DFMiniMp3 Instance
static DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(mySoftwareSerial);

// Create Button Instances
Button pauseButton(PIN_BTN_PLAY);
Button upButton(PIN_BTN_UP);
Button downButton(PIN_BTN_DOWN);


void setup()
{

  // Serial debug output
  Serial.begin(115200);
  Serial.println(F("TonUINO Version 2.0"));
  Serial.println(F("(c) Thorsten Voß"));

  // Init DFPlayer Mini
  setupPlayer();

  // Init NFC Reader
  setupCardReader();

  // Init Power Save
  setupPower();

}



void loop() {

  loopPower();

  loopPlayer();
  
  loopCardReader();

}


