

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

  volumeHoldTime = LONG_PRESS;
  ignoreVolumeHold = false;
  
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
          powerMp3TimerEnable();
        } else {
          powerMp3WakeUp();
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

    if (!ignoreVolumeHold && upButton.pressedFor(volumeHoldTime)) {

      if (volumeHoldTime >= 5 * LONG_PRESS) {
        volumeHoldTime += 125;
      } else if (volumeHoldTime >= 3 * LONG_PRESS) {
        volumeHoldTime += 250;
      } else {
        volumeHoldTime += 500;
      }
      
      if (volume < min(30, PLAYER_VOL_MAX)) {
        volume++;
        Serial.print(F("Volume Up "));
        Serial.println(volume);
        mp3.setVolume(volume); 
      } else {
        Serial.print(F("Volume MAX "));
        Serial.println(volume);
        mp3.playAdvertisement(404);
        ignoreVolumeHold = true;
      }
      
      ignoreUpButton = true;
      
    } else if (upButton.wasReleased()) {
      
      if (!ignoreUpButton) {
        nextTrack(random(65536));
      } else {
        ignoreUpButton = false;
        ignoreVolumeHold = false;
        volumeHoldTime = LONG_PRESS;
      }
      
    } else if (!ignoreVolumeHold && downButton.pressedFor(volumeHoldTime)) {

      if (volumeHoldTime >= 5 * LONG_PRESS) {
        volumeHoldTime += 125;
      } else if (volumeHoldTime >= 3 * LONG_PRESS) {
        volumeHoldTime += 250;
      } else {
        volumeHoldTime += 500;
      }
      
      if (volume > max(0, PLAYER_VOL_MIN)) {
        volume--;
        Serial.print(F("Volume Down "));
        Serial.println(volume);
        mp3.setVolume(volume);
      } else {
        Serial.print(F("Volume MIN "));
        Serial.println(volume);
        mp3.playAdvertisement(404);
        ignoreVolumeHold = true;
      }
      ignoreDownButton = true;
      
    } else if (downButton.wasReleased()) {
      
      if (!ignoreDownButton) {
        previousTrack();
      } else {
        ignoreDownButton = false;
        ignoreVolumeHold = false;
        volumeHoldTime = LONG_PRESS;
      }
    }
    // Ende der Buttons

  } else {
    // no card
    ignoreVolumeHold = false;
    volumeHoldTime = LONG_PRESS;

#ifdef SOUND_BOARD    
    if (pauseButton.wasPressed()) {
      powerMp3WakeUp();
      mp3.playMp3FolderTrack(1000);
      playerReady = false;
    } else if (downButton.wasPressed()) {
      powerMp3WakeUp();
      mp3.playMp3FolderTrack(1001);
      playerReady = false;
    } else if (upButton.wasPressed()) {
      powerMp3WakeUp();
      mp3.playMp3FolderTrack(1002);
      playerReady = false;
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
    powerMp3TimerEnable();
    return;
  }

  if (!hasCard) {
    return;
  }

  if (playMode == pmRadioPlay) {
    
    Serial.println(F("Mode Radio Play (single) -> ignore nextTrack"));
    powerMp3TimerEnable();
  
  } else if (playMode == pmAlbum) {
  
    if (currentTrack < numTracksInFolder) {
      currentTrack++;
    } else {
      // repeat = true
      currentTrack = 1;
    }
    Serial.print(F("Mode Album (continue, repeat all) -> nextTrack: "));
    Serial.println(currentTrack);
    powerMp3WakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);    
  
  } else if (playMode == pmParty) {
  
    uint16_t oldTrack = currentTrack;
    // random's upper bound is excluded so we shuffle with one track too few
    currentTrack = random(1, numTracksInFolder);
    // that way we exclude the previously played track
    if (currentTrack >= oldTrack) currentTrack++;
    Serial.print(F("Mode Party (shuffle) -> nextTrack: "));
    Serial.println(currentTrack);
    powerMp3WakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
  
  } else if (playMode == pmSingle) {
    
    Serial.println(F("Mode Single -> ignore nextTrack"));
    powerMp3TimerEnable();
  
  } else if (playMode == pmAudioBook) {
    
    if (currentTrack < numTracksInFolder) {
      currentTrack++;
      Serial.print(F("Mode Audio Book -> save progress -> nextTrack"));
      Serial.println(currentTrack);
      powerMp3WakeUp();
      mp3.playFolderTrack(playFolder, currentTrack);
      // Save progress to EEPROM
      EEPROM.write(playFolder, currentTrack);
    } else {
      // Reset progress
      EEPROM.write(playFolder, 1);
      currentTrack = 1;
      powerMp3TimerEnable();
    }
    
  }
  
}

static void previousTrack()
{

  if (playMode == pmRadioPlay) {

    Serial.println(F("Mode Radio Play -> Play track from the beginning"));
    powerMp3WakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
  
  } else if (playMode == 2) {
  
    Serial.println(F("Mode Album -> Previous Track"));
    if (currentTrack > 1) {
      currentTrack--;
    }
    powerMp3WakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
  
  } else if (playMode == pmParty) {
  
    Serial.println(F("Mode Party -> Play track from the beginning"));
    powerMp3WakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
  
  } else if (playMode == pmSingle) {
  
    Serial.println(F("Mode Single -> Play track from the beginning"));
    powerMp3WakeUp();
    mp3.playFolderTrack(playFolder, currentTrack);
  
  } else if (playMode == pmAudioBook) {
  
    Serial.println(F("Mode Audio Book -> save progress -> previous track"));
    if (currentTrack > 1) {
      currentTrack--;
    }
    powerMp3WakeUp();
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
    powerMp3WakeUp();
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
    powerMp3WakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmAlbum) {
  
    Serial.println(F("Mode Album (continue, repeat all) -> Play folder, track 1"));
    currentTrack = 1;
    powerMp3WakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);

  } else if (cardCurrent.mode == pmParty) {
    
    currentTrack = random(1, numTracksInFolder + 1);
    Serial.print(F("Mode Party (shuffle) -> Play random track from folder: "));
    Serial.println(currentTrack);
    powerMp3WakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmSingle) {

    currentTrack = cardCurrent.special;
    Serial.print(F("Mode Single -> Play file "));
    Serial.println(currentTrack);
    powerMp3WakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmAudioBook) {

    currentTrack = EEPROM.read(cardCurrent.folder);
    if (currentTrack < 1 || currentTrack > numTracksInFolder) {
      currentTrack = 1;
      EEPROM.write(cardCurrent.folder, currentTrack);
    }
    Serial.print(F("Mode Audio Book -> Play folder from saved progress: "));
    Serial.println(currentTrack);
    powerMp3WakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  }
  
}



