#include <DFMiniMp3.h>
#include <EEPROM.h>
#include <JC_Button.h>
#include <MFRC522.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "PlayModes.h"

// DFPlayer Mini
SoftwareSerial mySoftwareSerial(2, 3); // RX, TX
uint16_t numTracksInFolder;
uint16_t currentTrack;


// this object stores nfc tag data
struct nfcTagObject {
  uint32_t cookie;
  uint8_t version;
  uint8_t folder;
  uint8_t mode;
  uint8_t special;
};

nfcTagObject cardCurrent;

static void nextTrack(uint16_t track);

int voiceMenu(int numberOfOptions, int startMessage, int messageOffset, bool preview, int previewFromFolder);

bool knownCard = false;
uint8_t knownFolder = 0;
uint8_t knownMode = 0;
uint8_t knownSpecial = 0;

// implement a notification class,
// its member methods will get called
//
class Mp3Notify {
public:
  static void OnError(uint16_t errorCode) {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print(F("Mp3Notify: Error "));
    Serial.println(errorCode);
  }
  static void OnPlayFinished(uint16_t track) {
    Serial.print(F("Mp3Notify: Play finished "));
    Serial.println(track);
    delay(100);
    nextTrack(track);
  }
  static void OnCardOnline(uint16_t code) {
    Serial.println(F("Mp3Notify: SD card online"));
  }
  static void OnCardInserted(uint16_t code) {
    Serial.println(F("Mp3Notify: SD card inserted"));
  }
  static void OnCardRemoved(uint16_t code) {
    Serial.println(F("Mp3Notify: SD card removed"));
  }
  static void OnUsbOnline(uint16_t code) {
      Serial.println(F("Mp3Notify: USB online"));
  }
  static void OnUsbInserted(uint16_t code) {
      Serial.println(F("Mp3Notify: USB inserted"));
  }
  static void OnUsbRemoved(uint16_t code) {
    Serial.println(F("Mp3Notify: USB removed"));
  }
};

static DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(mySoftwareSerial);

// Leider kann das Modul keine Queue abspielen.
static uint16_t _lastTrackFinished;
static void nextTrack(uint16_t track) {
  if (track == _lastTrackFinished) {
    return;
  }
  _lastTrackFinished = track;
   
  if (knownCard == false) {
    // Wenn eine neue Karte angelernt wird soll das Ende eines Tracks nicht
    // verarbeitet werden
    return;
  }

  if (cardCurrent.mode == pmRadioPlay) {
    
    Serial.println(F("Mode Radio Play (single) -> ignore nextTrack"));
    setStandByTimer();
  
  } else if (cardCurrent.mode == pmAlbum) {
  
    if (currentTrack < numTracksInFolder) {
      currentTrack++;
    } else {
      // repeat = true
      currentTrack = 1;
    }
    Serial.print(F("Mode Album (continue, repeat all) -> nextTrack: "));
    Serial.println(currentTrack);
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);    
  
  } else if (cardCurrent.mode == pmParty) {
  
    uint16_t oldTrack = currentTrack;
    // random's upper bound is excluded so we shuffle with one track too few
    currentTrack = random(1, numTracksInFolder);
    // that way we exclude the previously played track
    if (currentTrack >= oldTrack) currentTrack++;
    Serial.print(F("Mode Party (shuffle) -> nextTrack: "));
    Serial.println(currentTrack);
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmSingle) {
    
    Serial.println(F("Mode Single -> ignore nextTrack"));
    setStandByTimer();
  
  } else if (cardCurrent.mode == pmAudioBook) {
    
    if (currentTrack < numTracksInFolder) {
      currentTrack++;
      Serial.print(F("Mode Audio Book -> save progress -> nextTrack"));
      Serial.println(currentTrack);
      wakeUp();
      mp3.playFolderTrack(cardCurrent.folder, currentTrack);
      // Save progress to EEPROM
      EEPROM.write(cardCurrent.folder, currentTrack);
    } else {
      // Reset progress
      EEPROM.write(cardCurrent.folder, 1);
      currentTrack = 1;
      setStandByTimer();
    }
    
  }
  
}

static void previousTrack()
{

  if (cardCurrent.mode == pmRadioPlay) {

    Serial.println(F("Mode Radio Play -> Play track from the beginning"));
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == 2) {
  
    Serial.println(F("Mode Album -> Previous Track"));
    if (currentTrack > 1) {
      currentTrack--;
    }
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmParty) {
  
    Serial.println(F("Mode Party -> Play track from the beginning"));
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmSingle) {
  
    Serial.println(F("Mode Single -> Play track from the beginning"));
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmAudioBook) {
  
    Serial.println(F("Mode Audio Book -> save progress -> previous track"));
    if (currentTrack > 1) {
      currentTrack--;
    }
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
    // Save progress to EEPROM
    EEPROM.write(cardCurrent.folder, currentTrack);
 
  }

}


/**
 * Power Saving
 */

unsigned long sleepAtMillis = 0;

void setStandByTimer()
{
  sleepAtMillis = millis() + 1200000; // 20 minutes 20 * 60 * 1000
  Serial.print(F("setStandByTimer "));
  Serial.println(sleepAtMillis);
}

void disableStandByTimer()
{
  sleepAtMillis = 0;
  Serial.println(F("disableStandByTimer"));
}

bool isSleeping = false;

void checkStandByTimer()
{
  if (!isSleeping && sleepAtMillis != 0 && millis() > sleepAtMillis) {
    Serial.println(F("Enter sleep mode..."));
    mp3.sleep();
    disableStandByTimer();
    isSleeping = true;
  }
}

void wakeUp()
{
  disableStandByTimer();
  if (isSleeping) {
    Serial.println(F("Wake up from sleep mode..."));
    mp3.setPlaybackSource(DfMp3_PlaySource_Sd);
    isSleeping = false;
  }
}


/**
 * NFC Card Reader
 */

// MFRC522
#define RST_PIN 9                 // Configurable, see typical pin layout above
#define SS_PIN 10                 // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522
MFRC522::MIFARE_Key key;
bool successRead;
byte sector = 1;
byte blockAddr = 4;
byte trailerBlock = 7;
MFRC522::StatusCode status;


#define buttonPause A0
#define buttonUp A1
#define buttonDown A2
#define busyPin 4

#define LONG_PRESS 1000

Button pauseButton(buttonPause);
Button upButton(buttonUp);
Button downButton(buttonDown);
bool ignorePauseButton = false;
bool ignoreUpButton = false;
bool ignoreDownButton = false;

uint8_t numberOfCards = 0;

#define MIN_VOLUME 1
#define MAX_VOLUME 20
uint8_t volume = 10;

bool isPlaying()
{
  return !digitalRead(busyPin);
}

void setup()
{

  Serial.begin(115200); // Es gibt ein paar Debug Ausgaben über die serielle
                        // Schnittstelle
  randomSeed(analogRead(A0)); // Zufallsgenerator initialisieren

  Serial.println(F("TonUINO Version 2.0"));
  Serial.println(F("(c) Thorsten Voß"));

  // Knöpfe mit PullUp
  pinMode(buttonPause, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);

  // Busy Pin
  pinMode(busyPin, INPUT);

  // DFPlayer Mini initialisieren
  mp3.begin();
  mp3.setVolume(volume);

  // NFC Leser initialisieren
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  mfrc522
      .PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader

      
    // Prepare the key (used both as key A and as key B)
    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // RESET --- ALLE DREI KNÖPFE BEIM STARTEN GEDRÜCKT HALTEN -> alle bekannten
  // Karten werden gelöscht
  if (digitalRead(buttonPause) == LOW && digitalRead(buttonUp) == LOW &&
      digitalRead(buttonDown) == LOW) {
    Serial.println(F("Reset -> EEPROM wird gelöscht"));
    for (int i = 0; i < EEPROM.length(); i++) {
      EEPROM.write(i, 0);
    }
  }

  setStandByTimer();

}

/**
 * Stop When Card Away 
 */
static bool hasCard = false;
static byte lastCardUid[4];
static byte retries;
static bool lastCardWasUL;

// no change detected since last pollCard() call
#define PCS_NO_CHANGE 0

// card with new UID detected (had no card or other card before)
#define PCS_NEW_CARD 1

// card is not reachable anymore
#define PCS_CARD_GONE 2

// periodic MFRC reads are necessary to determine card changes 
byte pollCard()
{
  const byte maxRetries = 2;

  if (hasCard) {
    
    // perform a dummy read command just to see whether the card is in range
    byte buffer[18];
    byte size = sizeof(buffer);
    
    if (mfrc522.MIFARE_Read(lastCardWasUL ? 8 : blockAddr, buffer, &size) != MFRC522::STATUS_OK) {
      if (retries > 0) {
        retries--;
      } else {
        Serial.println(F("Card has gone!"));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        hasCard = false;
        return PCS_CARD_GONE;
      }
    } else {
      retries = maxRetries;
    }
 
  } else {
    
    // get new card info, if any
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial() && readCard(&cardCurrent)) {
      // remember that card
      memcpy(lastCardUid, mfrc522.uid.uidByte, 4);
      lastCardWasUL = mfrc522.PICC_GetType(mfrc522.uid.sak) == MFRC522::PICC_TYPE_MIFARE_UL;
      retries = maxRetries;
      hasCard = true;
      return PCS_NEW_CARD;
    }
    
  }
  
  return PCS_NO_CHANGE;
}


void handleCardReader()
{
  // poll card only every 100ms
  static uint16_t lastCardPoll = 0;
  uint16_t now = millis();

  uint16_t interval = isSleeping ? 1500 : 150;
  
  if (static_cast<uint16_t>(now - lastCardPoll) < interval) {
    return;
  }
  
  lastCardPoll = now;
  switch (pollCard()) {
    case PCS_NEW_CARD:
      onNewCard();
      break;
    case PCS_CARD_GONE:
      mp3.pause();
      setStandByTimer();
      break;    
  }

}

/**
 * Called when RFID Card is detected
 */
void onNewCard()
{  

  if (cardCurrent.cookie != 322417479 || cardCurrent.folder == 0 || cardCurrent.mode == 0) {

    // card is not configured
    knownCard = false;
    knownFolder = 0;
    knownMode = 0;
    knownSpecial = 0;
    setupCard();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  
  }

  if (!isPlaying() && knownCard == true && knownFolder == cardCurrent.folder && knownMode == cardCurrent.mode && knownSpecial == cardCurrent.special) {
   
    // continue playback
    Serial.println(F("Card is back! Continue..."));
    wakeUp();
    mp3.start(); 
    return;
        
  }

  // play different card than before
  knownCard = true;
  knownFolder = cardCurrent.folder;
  knownMode = cardCurrent.mode;
  knownSpecial = cardCurrent.special;
      
  _lastTrackFinished = 0;
  numTracksInFolder = mp3.getFolderTrackCount(cardCurrent.folder);
  Serial.print(numTracksInFolder);
  Serial.print(F(" Files in Folder "));
  Serial.println(cardCurrent.folder);

  if (cardCurrent.mode == pmRadioPlay) {
    
    currentTrack = random(1, numTracksInFolder + 1);
    Serial.print(F("Mode Radio Play (shuffle, single) -> Play random track: "));
    Serial.println(currentTrack);
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmAlbum) {
  
    Serial.println(F("Mode Album (continue, repeat all) -> Play folder, track 1"));
    currentTrack = 1;
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);

  } else if (cardCurrent.mode == pmParty) {
    
    currentTrack = random(1, numTracksInFolder + 1);
    Serial.print(F("Mode Party (shuffle) -> Play random track from folder: "));
    Serial.println(currentTrack);
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmSingle) {

    currentTrack = cardCurrent.special;
    Serial.print(F("Mode Single -> Play file "));
    Serial.println(currentTrack);
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  } else if (cardCurrent.mode == pmAudioBook) {

    currentTrack = EEPROM.read(cardCurrent.folder);
    Serial.print(F("Mode Audio Book -> Play folder from saved progress: "));
    Serial.println(currentTrack);
    wakeUp();
    mp3.playFolderTrack(cardCurrent.folder, currentTrack);
  
  }
  
}


void loop() {

  checkStandByTimer();
  
  // calling mp3.loop() periodically allows for notifications 
  // to be handled without interrupts
  mp3.loop();

  if (hasCard) {
  
    // Buttons werden nun über JS_Button gehandelt, dadurch kann jede Taste
    // doppelt belegt werden
    pauseButton.read();
    upButton.read();
    downButton.read();

    if (pauseButton.wasReleased()) {
      if (ignorePauseButton == false) {
        if (isPlaying()) {
          mp3.pause();
          setStandByTimer();
        } else {
          wakeUp();
          mp3.start();
        }
      }
      ignorePauseButton = false;
    } else if (pauseButton.pressedFor(LONG_PRESS) &&
               ignorePauseButton == false) {
      if (isPlaying())
        mp3.playAdvertisement(currentTrack);
      else {
        knownCard = false;
        mp3.playMp3FolderTrack(800);
        Serial.println(F("Reset card..."));
        resetCard();
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
      }
      ignorePauseButton = true;
    }

    if (upButton.pressedFor(LONG_PRESS)) {
      
      if (volume < MAX_VOLUME) {
        Serial.println(F("Volume Up"));
        mp3.setVolume(++volume); 
      } else {
        Serial.println(F("Volume MAX"));
      }
      ignoreUpButton = true;
    } else if (upButton.wasReleased()) {
      if (!ignoreUpButton)
        nextTrack(random(65536));
      else
        ignoreUpButton = false;
    }

    if (downButton.pressedFor(LONG_PRESS)) {
      if (volume > MIN_VOLUME) {
        Serial.println(F("Volume Down"));
        mp3.setVolume(--volume);
      } else {
        Serial.println(F("Volume MIN"));
      }
      ignoreDownButton = true;
    } else if (downButton.wasReleased()) {
      if (!ignoreDownButton)
        previousTrack();
      else
        ignoreDownButton = false;
    }
    // Ende der Buttons

  }
  
  // prüfe ob eine neue Karte da ist oder die alte entfernt wurde
  handleCardReader();

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

int voiceMenu(int numberOfOptions, int startMessage, int messageOffset, bool preview = false, int previewFromFolder = 0)
{
  int returnValue = 0;

  // poll card only every 100ms
  
  wakeUp();

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

void resetCard()
{
  
  do {
    pauseButton.read();
    upButton.read();
    downButton.read();

    if (upButton.wasReleased() || downButton.wasReleased()) {
      Serial.println(F("Cancelled"));
      mp3.playMp3FolderTrack(802);
      waitMilliseconds(2000);
      return;
    }
    
  } while (pollCard() != PCS_NEW_CARD);

  setupCard();
}


void setupCard()
{

  mp3.pause();
  Serial.println(F("Configure card"));

  // ask folder
  cardCurrent.folder = voiceMenu(99, 300, 0, true);

  if (cardCurrent.folder < 1) {
    cardCurrent.folder = 0;
    Serial.println(F("Cancelled"));
    mp3.playMp3FolderTrack(401);
    waitMilliseconds(2000);
    return;
  }

  // ask mode
  cardCurrent.mode = voiceMenu(6, 310, 310);
    
  if (cardCurrent.mode < 1) {
    cardCurrent.mode = 0;
    Serial.println(F("Cancelled"));
    mp3.playMp3FolderTrack(401);
    waitMilliseconds(2000);
    return;
  }

  // Set progress = 1 (mode audio book)
  EEPROM.write(cardCurrent.folder,1);

  if (cardCurrent.mode == pmSingle) {

    // ask file
    cardCurrent.special = voiceMenu(mp3.getFolderTrackCount(cardCurrent.folder), 320, 0, true, cardCurrent.folder);
    if (cardCurrent.special < 1) {
      cardCurrent.special = 0;
      Serial.println(F("Cancelled"));
      mp3.playMp3FolderTrack(401);
      waitMilliseconds(2000);
      return;
    }

  }
  
  if (cardCurrent.mode == pmAdmin) {
    // admin function
    cardCurrent.special = voiceMenu(3, 320, 320);
    if (cardCurrent.special < 1) {
      cardCurrent.special = 0;
      Serial.println(F("Cancelled"));
      mp3.playMp3FolderTrack(401);
      waitMilliseconds(2000);
      return;
    }
  }

  // configuration complete, write to card...
  mp3.pause();
  writeCard(cardCurrent);
}


bool readCard(nfcTagObject *nfcTag)
{
  bool returnValue = true;
  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  // Show the whole sector as it currently is
  Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // Read data from the block
  Serial.print(F("Reading data from block "));
  Serial.print(blockAddr);
  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    returnValue = false;
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("Data in block "));
  Serial.print(blockAddr);
  Serial.println(F(":"));
  dump_byte_array(buffer, 16);
  Serial.println();
  Serial.println();

  uint32_t tempCookie;
  tempCookie = (uint32_t)buffer[0] << 24;
  tempCookie += (uint32_t)buffer[1] << 16;
  tempCookie += (uint32_t)buffer[2] << 8;
  tempCookie += (uint32_t)buffer[3];

  nfcTag->cookie = tempCookie;
  nfcTag->version = buffer[4];
  nfcTag->folder = buffer[5];
  nfcTag->mode = buffer[6];
  nfcTag->special = buffer[7];

  return returnValue;
}

void writeCard(nfcTagObject nfcTag)
{
  MFRC522::PICC_Type mifareType;
  byte buffer[16] = {0x13, 0x37, 0xb3, 0x47, // 0x1337 0xb347 magic cookie to
                                             // identify our nfc tags
                     0x01,                   // version 1
                     nfcTag.folder,          // the folder picked by the user
                     nfcTag.mode,    // the playback mode picked by the user
                     nfcTag.special, // track or function for admin cards
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  byte size = sizeof(buffer);

  mifareType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  // Authenticate using key B
  Serial.println(F("Authenticating again using key B..."));
  status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    mp3.playMp3FolderTrack(401);
    waitMilliseconds(2000);
    return;
  }

  // Write data to the block
  Serial.print(F("Writing data into block "));
  Serial.print(blockAddr);
  Serial.println(F(" ..."));
  dump_byte_array(buffer, 16);
  Serial.println();
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(blockAddr, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    mp3.playMp3FolderTrack(401);
  } else {
    mp3.playMp3FolderTrack(400);
  }
  Serial.println();
  waitMilliseconds(2000);
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

