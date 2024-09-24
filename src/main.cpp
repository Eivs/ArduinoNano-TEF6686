#include <Arduino.h>
#include <TEF6686.h>

TEF6686 tef;
unsigned long lastRDSTime;

void setup()
{
  Serial.begin(9600);

  tef.Init();
  tef.Audio_Set_Mute(0);             // disable mute
  tef.Tune_To(tef.MODULE_FM, 10460); // tune FM to 104,6
}

void loop()
{
  if (millis() - 500 > lastRDSTime)
  {
    tef.UpdateRDSStatus(); // should be called at least every 87ms
    tef.UpdateQualityStatus();
    char buffer[200];
    sprintf(buffer, "Freq: %i.%i, MS: %i, TA: %i, PTY: %s, PS: %s,  RT: %s, Quality: %i\n",
            tef.Currentfreq / 100,
            tef.Currentfreq % 100,
            tef.rdsData.ms,
            tef.rdsData.ta,
            ptyLabels[tef.rdsData.pty],
            tef.rdsData.psText,
            tef.rdsData.rtText,
            tef.quality);
    // display.clearDisplay();
    // display.setTextSize(1);
    // display.setTextColor(SH110X_WHITE);
    // display.setCursor(0, 0);
    // display.println(buffer);
    Serial.println(buffer);

    lastRDSTime = millis();
  }
}
