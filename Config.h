/**
 * JoniBOX, based on TonUINO
 * @see https://github.com/xfjx/TonUINO
 * @see https://www.voss.earth/tonuino/
 *
 * Config
 */
#ifndef CONFIG_H
#define CONFIG_H


// PIN CONFIG

//  DFPlayer wiring (TonUINO default)
#define PIN_MP3_RX 2
#define PIN_MP3_TX 3
#define PIN_MP3_BUSY 4

//  alternative DFPlayer wiring
//#define PIN_MP3_RX 4
//#define PIN_MP3_TX 5
//#define PIN_MP3_BUSY 6


//  button inputs (TonUINO default)
#define PIN_BTN_PLAY A0
#define PIN_BTN_UP A1
#define PIN_BTN_DOWN A2

//  MFRC522 wiring (TonUINO default)
#define PIN_CR_RST 9
#define PIN_CR_SS 10

//  led output
//#define PIN_LED 5

// BUTTON CONFIG

//  swap short/long press function (track/volume)
//#define SWAP_TRACK_VOLUME

//  min time (ms) for long press actions
#define LONG_PRESS 1000

//  Without card, buttons can play track 1000/1001/1002 from mp3 folder.
//  Uncomment following line to enable this feature
//#define SOUND_BOARD



// PLAYER CONFIG

// Define min, max and startup volume
// range: 0 (off) ... 30
#define PLAYER_VOL_MIN 1
#define PLAYER_VOL_MAX 20
#define PLAYER_VOL_START 10


// POWER CONFIG

//  After this time (ms) mp3 module will be send to sleep mode (Pause position will be lost)
#define POWER_TIMEOUT 300000 // 5 minutes 5 * 60 * 1000
#define POWER_LED_DIM 3


#endif
