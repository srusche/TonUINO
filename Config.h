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
//#define PIN_MP3_RX 2
//#define PIN_MP3_TX 3
//#define PIN_MP3_BUSY 4
#define PIN_MP3_RX 4
#define PIN_MP3_TX 5
#define PIN_MP3_BUSY 6


#define PIN_BTN_PLAY A0
#define PIN_BTN_UP A1
#define PIN_BTN_DOWN A2


#define PIN_CR_RST 9                 // Configurable, see typical pin layout above
#define PIN_CR_SS 10                 // Configurable, see typical pin layout above

// BUTTON CONFIG
#define LONG_PRESS 1000

// PLAYER CONFIG
#define PLAYER_VOL_MIN 1
#define PLAYER_VOL_MAX 20
#define PLAYER_VOL_START 10

// POWER CONFIG
#define POWER_TIMEOUT 1200000 // 20 minutes 20 * 60 * 1000

#endif
