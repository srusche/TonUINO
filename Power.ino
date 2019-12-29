/**
 * JoniBOX, based on TonUINO
 * @see https://github.com/xfjx/TonUINO
 * @see https://www.voss.earth/tonuino/
 * 
 * Power Save Helpers
 */

void setupPower()
{
  powerSleepTime = 0;
  powerState = POWER_AWAKE;

  powerTimerEnable();
}

void loopPower()
{
  if (!powerState && powerSleepTime != 0 && millis() > powerSleepTime) {
    Serial.println(F("Power: Enter sleep mode..."));
    mp3.sleep();
    powerTimerDisable();
    powerState = POWER_SLEEPING;
  }
}

void powerTimerEnable()
{
  powerSleepTime = millis() + POWER_TIMEOUT;
  Serial.print(F("Power: Timer enabled "));
  Serial.println(powerSleepTime);
}

void powerTimerDisable()
{
  if (powerSleepTime == 0) {
    return;
  }
  powerSleepTime = 0;
  Serial.println(F("Power: Timer disabled"));
}

void powerWakeUp()
{
  powerTimerDisable();
  if (powerState != POWER_AWAKE) {
    Serial.println(F("Power: Wake up from sleep mode..."));
    mp3.setPlaybackSource(DfMp3_PlaySource_Sd);
    powerState = POWER_AWAKE;
  }
}

