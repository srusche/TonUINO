/**
 * JoniBOX, based on TonUINO
 * @see https://github.com/xfjx/TonUINO
 * @see https://www.voss.earth/tonuino/
 * 
 * Card Config
 */

int cardConfigMenu(int numberOfOptions, int startMessage, int messageOffset, bool preview = false, int previewFromFolder = 0)
{
  int returnValue = 0;

  // poll card only every 100ms
  
  powerTimerDisable();

  if (startMessage != 0) {
    mp3.playMp3FolderTrack(startMessage);
  }

  uint8_t lastCardPoll = 0;
  
  do {

    uint8_t now = millis();
    
    if (static_cast<uint8_t>(now - lastCardPoll) >= 100) {
      lastCardPoll = now;
      if (pollCard() != PCS_NO_CHANGE) {
        // card has changed, stop configuration
        return 0;
      }
    }
    
    pauseButton.read();
    upButton.read();
    downButton.read();
    mp3.loop();
    
    if (pauseButton.wasPressed() && returnValue != 0) {
      
      return returnValue;
    
    } else if (upButton.pressedFor(LONG_PRESS)) {

      // up long press: increase +10
      returnValue = min(returnValue + 10, numberOfOptions);

      // announce new value
      mp3.playMp3FolderTrack(messageOffset + returnValue);
      waitMilliseconds(1000);
      if (preview) {
        do {
          waitMilliseconds(10);
        } while (isPlaying());
        if (previewFromFolder == 0) {
          mp3.playFolderTrack(returnValue, 1);
        } else {
          mp3.playFolderTrack(previewFromFolder, returnValue);
        }
      }
      ignoreUpButton = true;
      
    } else if (upButton.wasReleased()) {
      
      if (!ignoreUpButton) {
        
        // up short press, increase +1
        returnValue = min(returnValue + 1, numberOfOptions);
        mp3.playMp3FolderTrack(messageOffset + returnValue);
        waitMilliseconds(1000);
        if (preview) {
          do {
            waitMilliseconds(10);
          } while (isPlaying());
          if (previewFromFolder == 0) {
            mp3.playFolderTrack(returnValue, 1);
          } else {
            mp3.playFolderTrack(previewFromFolder, returnValue);
          }
        }
        
      } else {
        ignoreUpButton = false;
      }
      
    } else if (downButton.pressedFor(LONG_PRESS)) {

      // down long press: decrease -10
      returnValue = max(returnValue - 10, 1);
      mp3.playMp3FolderTrack(messageOffset + returnValue);
      waitMilliseconds(1000);
      if (preview) {
        do {
          waitMilliseconds(10);
        } while (isPlaying());
        if (previewFromFolder == 0) {
          mp3.playFolderTrack(returnValue, 1);
        } else {
          mp3.playFolderTrack(previewFromFolder, returnValue);
        }
      }
      ignoreDownButton = true;
      
    } else if (downButton.wasReleased()) {
      
      if (!ignoreDownButton) {
        
        // down short press, decrease -1
        returnValue = max(returnValue - 1, 1);
        mp3.playMp3FolderTrack(messageOffset + returnValue);
        waitMilliseconds(1000);
        if (preview) {
          do {
            waitMilliseconds(10);
          } while (isPlaying());
          if (previewFromFolder == 0) {
            mp3.playFolderTrack(returnValue, 1);
          } else {
            mp3.playFolderTrack(previewFromFolder, returnValue);
          }
        }
        
      } else {
      
        ignoreDownButton = false;
      
      }
      
    }
    
  } while (true);

}

void cardConfigReset()
{
  
  do {
    pauseButton.read();
    upButton.read();
    downButton.read();

    if (upButton.wasReleased() || downButton.wasReleased()) {
      Serial.println(F("Cancelled"));
      mp3.playMp3FolderTrack(802);
      waitMilliseconds(1000);
      do {
        waitMilliseconds(10);
      } while (isPlaying());
      return;
    }
    
  } while (pollCard() != PCS_NEW_CARD);

  cardConfigStart();
}


void cardConfigStart()
{

  mp3.pause();
  Serial.println(F("Configure card"));

  // ask folder
  cardCurrent.folder = cardConfigMenu(99, 300, 0, true);

  if (cardCurrent.folder < 1) {
    cardCurrent.folder = 0;
    Serial.println(F("Cancelled"));
    mp3.playMp3FolderTrack(401);
    waitMilliseconds(1000);
    do {
      waitMilliseconds(10);
    } while (isPlaying());
    return;
  }

  // ask mode
  cardCurrent.mode = cardConfigMenu(6, 310, 310);
    
  if (cardCurrent.mode < 1) {
    cardCurrent.mode = 0;
    Serial.println(F("Cancelled"));
    mp3.playMp3FolderTrack(401);
    waitMilliseconds(1000);
    do {
      waitMilliseconds(10);
    } while (isPlaying());
    return;
  }

  // Set progress = 1 (mode audio book)
  EEPROM.write(cardCurrent.folder,1);

  if (cardCurrent.mode == pmSingle) {

    // ask file
    cardCurrent.special = cardConfigMenu(mp3.getFolderTrackCount(cardCurrent.folder), 320, 0, true, cardCurrent.folder);
    if (cardCurrent.special < 1) {
      cardCurrent.special = 0;
      Serial.println(F("Cancelled"));
      mp3.playMp3FolderTrack(401);
      waitMilliseconds(1000);
      do {
        waitMilliseconds(10);
      } while (isPlaying());
      return;
    }

  }
  
  if (cardCurrent.mode == pmAdmin) {
    // admin function
    cardCurrent.special = cardConfigMenu(3, 320, 320);
    if (cardCurrent.special < 1) {
      cardCurrent.special = 0;
      Serial.println(F("Cancelled"));
      mp3.playMp3FolderTrack(401);
      waitMilliseconds(1000);
      do {
        waitMilliseconds(10);
      } while (isPlaying());
      return;
    }
  }

  // configuration complete, write to card...
  mp3.pause();
  writeCard(cardCurrent);
}



