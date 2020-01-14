/**
 * JoniBOX, based on TonUINO
 * @see https://github.com/xfjx/TonUINO
 * @see https://www.voss.earth/tonuino/
 * 
 * Power Save Helpers
 */

void setupPower()
{
#ifdef PIN_LED
  pinMode(PIN_LED, OUTPUT);
  powerLedBrightness = 64;
  powerLedUp = true;
  analogWrite(PIN_LED, powerLedBrightness);
#endif
  powerSleepTime = 0;
 
  powerTimerEnable();
}

void loopPower()
{
#ifdef POWER_TIMEOUT
  if (powerSleepTime != 0 && millis() > powerSleepTime) {
    Serial.println(F("Power: Shutdown..."));

#ifdef PIN_LED
    analogWrite(PIN_LED, 0);
#endif

    mfrc522.PCD_AntennaOff();
    mfrc522.PCD_SoftPowerDown();
    mp3.sleep();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();  // Disable interrupts
    sleep_mode();
  
  }
#endif

#ifdef PIN_LED
    if (powerSleepTime > 0) {
      powerLedFade();
    } else if (powerLedBrightness != 255) {
      powerLedBrightness = 255;
      analogWrite(PIN_LED, powerLedBrightness);  
    }
#endif

}

void powerLedFade()
{
#ifdef PIN_LED
  static uint16_t lastLedDim = 0;
  uint16_t now = millis();

  if (static_cast<uint16_t>(now - lastLedDim) >= 15) {

    lastLedDim = now;

    if (powerLedUp && powerLedBrightness >= 255 - POWER_LED_DIM) {
      powerLedUp = false;
    } else if (!powerLedUp && powerLedBrightness <= 16 + POWER_LED_DIM) {
      powerLedUp = true;
    }

    if (powerLedUp) {
      powerLedBrightness += POWER_LED_DIM;
    } else {
      powerLedBrightness -= POWER_LED_DIM;
    }
    
    analogWrite(PIN_LED, powerLedBrightness);
    
  }
#endif
}

void powerTimerEnable()
{
#ifdef POWER_TIMEOUT
  powerSleepTime = millis() + POWER_TIMEOUT;
  Serial.print(F("Power: Timer enabled "));
  Serial.println(powerSleepTime);
#endif
}

void powerTimerDisable()
{
#ifdef POWER_TIMEOUT
  if (powerSleepTime == 0) {
    return;
  }
  powerSleepTime = 0;
  Serial.println(F("Power: Timer disabled"));
#endif
}


