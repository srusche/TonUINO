/**
 * JoniBOX, based on TonUINO
 * @see https://github.com/xfjx/TonUINO
 * @see https://www.voss.earth/tonuino/
 * 
 * Variables
 */
#ifndef VARIABLES_H
#define VARIABLES_H

/*
 * PlayModes
 * +-----+------------+--------+--------+---------+----------+
 * | Num | Name       | single | repeat | shuffle | progress |
 * +-----+------------+--------+--------+---------+----------|
 * |  1  | radio play | yes    | no     | yes     | no       |
 * |  2  | album      | no     | yes    | no      | no       |
 * |  3  | party      | no     | yes    | yes     | no       |
 * |  4  | single     | yes    | yes    | no      | no       |
 * |  5  | audio book | no     | no     | no      | yes      |
 * |  6  | admin      |        |        |         |          |
 * +-----+------------+--------+--------+---------+----------+
 */
#define pmRadioPlay 1
#define pmAlbum 2
#define pmParty 3
#define pmSingle 4
#define pmAudioBook 5
#define pmAdmin 6

/*
 * Player DFPlayer Mini
 */

class Mp3Notify {
  public:
    static void OnError(uint16_t errorCode);
    static void OnPlayFinished(uint16_t track);
    static void OnCardOnline(uint16_t code);
    static void OnCardInserted(uint16_t code);
    static void OnCardRemoved(uint16_t code);
    static void OnUsbOnline(uint16_t code);
    static void OnUsbInserted(uint16_t code);
    static void OnUsbRemoved(uint16_t code);
};



/**
 * Player Buttons
 */

bool ignorePauseButton;
bool ignoreUpButton;
bool ignoreDownButton;
unsigned long volumeHoldTime;
bool ignoreVolumeHold;



/*
 * Player Status
 */

uint16_t numTracksInFolder;
uint16_t currentTrack;
uint8_t volume;

bool playerReady;
uint8_t playFolder;
uint8_t playMode;
uint8_t playFile;


/**
 * Card Reader
 */


// this object stores nfc tag data
struct nfcTagObject {
  uint32_t cookie;
  uint8_t version;
  uint8_t folder;
  uint8_t mode;
  uint8_t special;
};


nfcTagObject cardCurrent;

/**
 * NFC Card Reader
 */


// MFRC522
static MFRC522::MIFARE_Key key;
static bool successRead;
static byte sector = 1;
static byte blockAddr = 4;
static byte trailerBlock = 7;
static MFRC522::StatusCode status;



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



/**
 * Power Save
 */
unsigned long powerMp3SleepTime;
bool powerMp3State;
unsigned long powerCardSleepTime;
bool powerCardState;

#define POWER_AWAKE false
#define POWER_SLEEPING true


#endif

