

void Mp3Notify::OnError(uint16_t errorCode) {
  // see DfMp3_Error for code meaning
  Serial.println();
  Serial.print(F("Mp3Notify: Error "));
  Serial.println(errorCode);
}
void Mp3Notify::OnPlayFinished(uint16_t track) {
  Serial.print(F("Mp3Notify: Play finished "));
  Serial.println(track);
  delay(100);
  nextTrack(track);
}
void Mp3Notify::OnCardOnline(uint16_t code) {
  Serial.println(F("Mp3Notify: SD card online"));
}
void Mp3Notify::OnCardInserted(uint16_t code) {
  Serial.println(F("Mp3Notify: SD card inserted"));
}
void Mp3Notify::OnCardRemoved(uint16_t code) {
  Serial.println(F("Mp3Notify: SD card removed"));
}
void Mp3Notify::OnUsbOnline(uint16_t code) {
    Serial.println(F("Mp3Notify: USB online"));
}
void Mp3Notify::OnUsbInserted(uint16_t code) {
    Serial.println(F("Mp3Notify: USB inserted"));
}
void Mp3Notify::OnUsbRemoved(uint16_t code) {
  Serial.println(F("Mp3Notify: USB removed"));
}


void playerSetupButtons()
{
  
  ignorePauseButton = false;
  ignoreUpButton = false;
  ignoreDownButton = false;
  ignoreUpLongButton = false;
  ignoreDownLongButton = false;
  
  pinMode(PIN_BTN_PLAY, INPUT_PULLUP);
  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);

}


void setupPlayer()
{
  playerSetupButtons();
  
  playerReady = false;
  playFolder = 0;
  playMode = 0;
  playFile = 0;

  // init random generator
  randomSeed(analogRead(A0));

  // mp3 module busy Pin
  pinMode(PIN_MP3_BUSY, INPUT);
  

  // setup DFMiniMp3
  mp3.begin();
  volume = PLAYER_VOL_START;
  mp3.setVolume(volume);
  
}

void loopPlayer()
{
  
  // calling mp3.loop() periodically allows for notifications 
  // to be handled without interrupts
  mp3.loop();

  // handle buttons
  pauseButton.read();
  upButton.read();
  downButton.read();

  if (hasCard) {
  
    if (pauseButton.wasReleased()) {
      if (ignorePauseButton == false) {
        if (isPlaying()) {
          mp3.pause();
          powerTimerEnable();
        } else {
          powerWakeUp();
          mp3.start();
        }
      }
      ignorePauseButton = false;
    } else if (pauseButton.pressedFor(LONG_PRESS) && ignorePauseButton == false) {
      if (isPlaying()) {
        mp3.playAdvertisement(currentTrack);
      } else {
        playerReady = false;
        mp3.playMp3FolderTrack(800);
        Serial.println(F("Reset card..."));
        cardConfigReset();
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
      }
      ignorePauseButton = true;
    }

    if (upButton.pressedFor(LONG_PRESS)) {
      
      if (volume < PLAYER_VOL_MAX) {
        Serial.println(F("Volume Up"));
        mp3.setVolume(++volume); 
      } else if (!ignoreUpLongButton) {
        Serial.println(F("Volume MAX"));
        mp3.playAdvertisement(404);
        ignoreUpLongButton = true;
      }
      ignoreUpButton = true;
    } else if (upButton.wasReleased()) {
      if (!ignoreUpButton) {
        nextTrack(random(65536));
      } else {
        ignoreUpButton = false;
        ignoreUpLongButton = false;
      }
    }

    if (downButton.pressedFor(LONG_PRESS)) {
      if (volume > PLAYER_VOL_MIN) {
        Serial.println(F("Volume Down"));
        mp3.setVolume(--volume);
      } else if (!ignoreDownLongButton) {
        Serial.println(F("Volume MIN"));
        mp3.playAdvertisement(404);
        ignoreDownLongButton = true;
      }
      ignoreDownButton = true;
    } else if (downButton.wasReleased()) {
      if (!ignoreDownButton) {
        previousTrack();
      } else {
        ignoreDownButton = false;
        ignoreDownLongButton = false;
      }
    }
    // Ende der Buttons

  } else {
    // no card

#ifdef SOUND_BOARD    
    if (pauseButton.wasPressed()) {
      powerWakeUp();
      mp3.playMp3FolderTrack(1000);
      waitPlaybackEnd();
      playerReady = false;
      powerTimerEnable();
    } else if (downButton.wasPressed()) {
      powerWakeUp();
      mp3.playMp3FolderTrack(1001);
      waitPlaybackEnd();
      playerReady = false;
      powerTimerEnable();
    } else if (upButton.wasPressed()) {
      powerWakeUp();
      mp3.playMp3FolderTrack(1002);
      waitPlaybackEnd();
      playerReady = false;
      powerTimerEnable();
    }
#endif

  }
}




// Leider kann das Modul keine Queue abspielen.
static uint16_t _lastTrackFinished;
static void nextTrack(uint16_t track) {
  if (track == _lastTrackFinished) {
    return;
  }
  _lastTrackFinished = track;
   
  if (playerReady == false) {
    // Wenn eine neue Karte angelernt wird soll das Ende eines Tracks nicht
    // verarbeitet werden
    return;
  }

  if (!hasCard) {
    return;
  }

  if (playMode == pmRadioPlay) {
    
    Serial.println(F("Mode Radio Play (single) -> ignore nextTrack"));
    powerTimerEnable();
  
  } else if (playMode == pmAlbum) {
  
    if (currentTrack < numTracksInFolder) {
      currentTrack++;
    } else {
      // repeat = true
      currentTrack = 1;
    }
    Serial.print(F("Mode Album (continue, repeat all) -> nextTrack: "));
    Serial.println(currentTrack);
    powerWakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);    
  
  } else if (playMode == pmParty) {
  
    uint16_t oldTrack = currentTrack;
    // random's upper bound is excluded so we shuffle with one track too few
    currentTrack = random(1, numTracksInFolder);
    // that way we exclude the previously played track
    if (currentTrack >= oldTrack) currentTrack++;
    Serial.print(F("Mode Party (shuffle) -> nextTrack: "));
    Serial.println(currentTrack);
    powerWakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
  
  } else if (playMode == pmSingle) {
    
    Serial.println(F("Mode Single -> ignore nextTrack"));
    powerTimerEnable();
  
  } else if (playMode == pmAudioBook) {
    
    if (currentTrack < numTracksInFolder) {
      currentTrack++;
      Serial.print(F("Mode Audio Book -> save progress -> nextTrack"));
      Serial.println(currentTrack);
      powerWakeUp();
      mp3.playFolderTrack(playFolder, currentTrack);
      // Save progress to EEPROM
      EEPROM.write(playFolder, currentTrack);
    } else {
      // Reset progress
      EEPROM.write(playFolder, 1);
      currentTrack = 1;
      powerTimerEnable();
    }
    
  }
  
}

static void previousTrack()
{

  if (playMode == pmRadioPlay) {

    Serial.println(F("Mode Radio Play -> Play track from the beginning"));
    powerWakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
  
  } else if (playMode == 2) {
  
    Serial.println(F("Mode Album -> Previous Track"));
    if (currentTrack > 1) {
      currentTrack--;
    }
    powerWakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
  
  } else if (playMode == pmParty) {
  
    Serial.println(F("Mode Party -> Play track from the beginning"));
    powerWakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
  
  } else if (playMode == pmSingle) {
  
    Serial.println(F("Mode Single -> Play track from the beginning"));
    powerWakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
  
  } else if (playMode == pmAudioBook) {
  
    Serial.println(F("Mode Audio Book -> save progress -> previous track"));
    if (currentTrack > 1) {
      currentTrack--;
    }
    powerWakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
    // Save progress to EEPROM
    EEPROM.write(playFolder, currentTrack);
 
  }

}




bool isPlaying()
{
  return !digitalRead(PIN_MP3_BUSY);
}

void waitMilliseconds(uint16_t msWait)
{
  uint32_t tStart = millis();
  
  while ((millis() - tStart) < msWait)
  {
    // calling mp3.loop() periodically allows for notifications 
    // to be handled without interrupts
    mp3.loop(); 
    delay(1);
  }
}

void waitPlaybackEnd()
{
  if (!isPlaying()) {
    // maybe playback did not start yet
    waitMilliseconds(1000);
  }
  do {
    waitMilliseconds(10);
  } while (isPlaying());
}


/**
 * Called when RFID Card is detected
 */
void onNewCard()
{  

  if (cardCurrent.cookie != 322417479 || cardCurrent.folder == 0 || cardCurrent.mode == 0) {

    // card is not configured
    playerReady = false;
    playFolder = 0;
    playMode = 0;
    playFile = 0;
    cardConfigStart();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  
  }

  if (!isPlaying() && playerReady == true && playFolder == cardCurrent.folder && playMode == cardCurrent.mode && playFile == cardCurrent.special) {
   
    // continue playback
    Serial.println(F("Card is back! Continue..."));
    powerWakeUp();
    mp3.start(); 
    return;
        
  }

  // play different card than before
  playerReady = true;
  playFolder = cardCurrent.folder;
  playMode = cardCurrent.mode;
  playFile = cardCurrent.special;
      
  _lastTrackFinished = 0;
  numTracksInFolder = mp3.getFolderTrackCount(cardCurrent.folder);
  Serial.print(numTracksInFolder);
  Serial.print(F(" Files in Folder "));
  Serial.println(cardCurrent.folder);

  if (cardCurrent.mode == pmRadioPlay) {
    
    currentTrack = random(1, numTracksInFolder + 1);
    Serial.print(F("Mode Radio Play (shuffle, single) -> Play random track: "));
    Serial.println(currentTrack);
    powerWakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmAlbum) {
  
    Serial.println(F("Mode Album (continue, repeat all) -> Play folder, track 1"));
    currentTrack = 1;
    powerWakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);

  } else if (cardCurrent.mode == pmParty) {
    
    currentTrack = random(1, numTracksInFolder + 1);
    Serial.print(F("Mode Party (shuffle) -> Play random track from folder: "));
    Serial.println(currentTrack);
    powerWakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmSingle) {

    currentTrack = cardCurrent.special;
    Serial.print(F("Mode Single -> Play file "));
    Serial.println(currentTrack);
    powerWakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmAudioBook) {

    currentTrack = EEPROM.read(cardCurrent.folder);
    if (currentTrack < 1 || currentTrack > numTracksInFolder) {
      currentTrack = 1;
      EEPROM.write(cardCurrent.folder, currentTrack);
    }
    Serial.print(F("Mode Audio Book -> Play folder from saved progress: "));
    Serial.println(currentTrack);
    powerWakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  }
  
}



