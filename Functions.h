/**
 * JoniBOX, based on TonUINO
 * @see https://github.com/xfjx/TonUINO
 * @see https://www.voss.earth/tonuino/
 * 
 * Functions
 */
#ifndef FUNCTIONS_H
#define FUNCTIONS_H


/**
 * Player Buttons
 */

void playerSetupButtons();


/**
 * Player Functions
 */

void setupPlayer();
void loopPlayer();

static void nextTrack(uint16_t track);
static void previousTrack();

bool isPlaying();

void waitMilliseconds(uint16_t msWait);
void onNewCard();

/**
 * NFC Card Reader
 */

void setupCardReader();

bool readCard(nfcTagObject *nfcTag);
void writeCard(nfcTagObject nfcTag);
void dump_byte_array(byte *buffer, byte bufferSize);



/**
 * Stop When Card Away 
 */

byte pollCard();
void loopCardReader();

/**
 * Card Config
 */

int cardConfigMenu(int numberOfOptions, int startMessage, int messageOffset, bool preview, int previewFromFolder);
void cardConfigReset();
void cardConfigStart();

/**
 * Power Save
 */
void setupPower();
void loopPower();

void powerMp3TimerEnable();
void powerMp3TimerDisable();
void powerMp3WakeUp();


void powerCardTimerEnable(bool renew = false);
void powerCardTimerDisable();
void powerCardWakeUp();


#endif

