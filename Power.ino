/**
 * JoniBOX, based on TonUINO
 * @see https://github.com/xfjx/TonUINO
 * @see https://www.voss.earth/tonuino/
 * 
 * Power Save Helpers
 */

void setupPower()
{
  powerMp3SleepTime = 0;
  powerMp3State = POWER_AWAKE;

  powerCardSleepTime = 0;
  powerCardState = POWER_AWAKE;

  powerMp3TimerEnable();
  powerCardTimerEnable();
}

void loopPower()
{
#ifdef POWER_TIMEOUT_MP3
  if (powerMp3State == POWER_AWAKE && powerMp3SleepTime != 0 && millis() > powerMp3SleepTime) {
    Serial.println(F("Power MP3: Enter sleep mode..."));
    mp3.setVolume(0);
    mp3.sleep();
    powerMp3TimerDisable();
    powerMp3State = POWER_SLEEPING;
  }
#endif

#ifdef POWER_TIMEOUT_CARD
  if (powerCardState == POWER_AWAKE && !hasCard && powerCardSleepTime != 0 && millis() > powerCardSleepTime) {
    Serial.println(F("Power Card: Antenna off..."));
    mfrc522.PCD_AntennaOff();
    mfrc522.PCD_SoftPowerDown();
    powerCardTimerDisable();
    powerCardState = POWER_SLEEPING;
  }
#endif
}

void powerMp3TimerEnable()
{
#ifdef POWER_TIMEOUT_MP3
  powerMp3SleepTime = millis() + POWER_TIMEOUT_MP3;
  Serial.print(F("Power MP3: Timer enabled "));
  Serial.println(powerMp3SleepTime);
#endif
}

void powerMp3TimerDisable()
{
#ifdef POWER_TIMEOUT_MP3
  if (powerMp3SleepTime == 0) {
    return;
  }
  powerMp3SleepTime = 0;
  Serial.println(F("Power MP3: Timer disabled"));
#endif
}

void powerMp3WakeUp()
{
#ifdef POWER_TIMEOUT_MP3
  powerMp3TimerDisable();
  if (powerMp3State != POWER_AWAKE) {
    Serial.println(F("Power MP3: Wake up from sleep mode..."));
    mp3.setPlaybackSource(DfMp3_PlaySource_Sd);
    mp3.setVolume(PLAYER_VOL_START);
    powerMp3State = POWER_AWAKE;
  }
#endif
}


void powerCardTimerEnable(bool renew)
{
#ifdef POWER_TIMEOUT_CARD
  if (!renew && powerCardSleepTime > 0) {
    return;
  }
  powerCardSleepTime = millis() + POWER_TIMEOUT_CARD;
  Serial.print(F("Power Card: Timer enabled "));
  Serial.println(powerCardSleepTime);
#endif
}

void powerCardTimerDisable()
{
#ifdef POWER_TIMEOUT_CARD
  if (powerCardSleepTime == 0) {
    return;
  }
  powerCardSleepTime = 0;
  Serial.println(F("Power Card: Timer disabled"));
#endif
}

void powerCardWakeUp()
{
#ifdef POWER_TIMEOUT_CARD
  powerCardTimerDisable();
  if (powerCardState != POWER_AWAKE) {
    Serial.println(F("Power Card: Antenna on..."));
    mfrc522.PCD_SoftPowerUp();
    mfrc522.PCD_AntennaOn();
    powerCardState = POWER_AWAKE;
  }
#endif
}

