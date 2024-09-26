#include <Arduino.h>
#include <TEF6686.h>

TEF6686 tef;
unsigned long lastRDSTime;

uint8_t DEVICE_ADDR = 0x65;
uint8_t MODULE_FM = 32;    // FM = FM radio reception
uint8_t MODULE_AM = 33;    // AM = LW, MW and SW radio reception
uint8_t MODULE_AUDIO = 48; // AUDIO = Audio processing
uint8_t MODULE_APPL = 64;  //  APPL = System and application control
uint16_t FREQMIN = 8700; // FM minimum frequency
uint16_t FREQMAX = 10800; // FM maximum frequency


void setup()
{
  Serial.begin(9600);

  tef.Init();
  // tef.Audio_Set_Mute(0);             // disable mute
  tef.Tune_To(MODULE_FM, 10460); // tune FM to 104,6
  tef.Set_Cmd(MODULE_APPL, 1, 1, 0);  // Device power up
  tef.Set_Cmd(MODULE_FM, 31, 1, 500);  // 50us for FM deemphasis time
  tef.Set_Cmd(MODULE_AUDIO, 11, 1, 0); //unmute
  int16_t volume = 50; // Default volume is 50%
  tef.Set_Cmd(MODULE_AUDIO, 10, 1, volume * 7 - 600);
  uint16_t REG_FREQ = 10460; // Default frequency is 104,60Mhz
  tef.Set_Cmd(32, 1, 2, 1, REG_FREQ);
}

void loop()
{
  if (millis() - 100 > lastRDSTime)
  {
    tef.UpdateRDSStatus(); // should be called at least every 87ms
    tef.UpdateQualityStatus();
    char buffer[200];
    sprintf(buffer, "Freq: %i.%i, MS: %i, TA: %i, PTY: %s, PS: %s,  RT: %s, Quality: %i\n",
            tef.Currentfreq / 100,
            tef.Currentfreq % 100,
            tef.rdsData.ms,
            tef.rdsData.ta,
            pgm_read_byte_near(ptyLabels[tef.rdsData.pty]),
            tef.rdsData.psText,
            tef.rdsData.rtText,
            tef.quality);
    Serial.println(buffer);

    lastRDSTime = millis();
  }
}
