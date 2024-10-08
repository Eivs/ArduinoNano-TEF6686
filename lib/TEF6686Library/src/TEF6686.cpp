#include <TEF6686.h>

void TEF6686::Write(uint8_t *buf, uint8_t len)
{
  Wire.beginTransmission(DEVICE_ADDR);
  for (int i = 0; i < len; i++)
    Wire.write(*buf++);
  Wire.endTransmission();
}

void TEF6686::Read(uint8_t *buf, uint8_t len)
{
  uint8_t lenrec = Wire.requestFrom(DEVICE_ADDR, len);
  for (int i = 0; i < lenrec; i++)
    *buf++ = Wire.read();
}

void TEF6686::Set_Cmd(uint8_t mdl, uint8_t cmd, int len, ...)
{
  uint8_t buf[31];
  uint16_t temp;
  va_list vArgs;
  va_start(vArgs, len);
  buf[0] = mdl;
  buf[1] = cmd;
  buf[2] = 1;
  for (uint8_t i = 0; i < len; i++)
  {
    temp = va_arg(vArgs, uint16_t);
    buf[3 + i * 2] = (uint8_t)(temp >> 8);
    buf[4 + i * 2] = (uint8_t)temp;
  }
  va_end(vArgs);
  Write(buf, len * 2 + 3);
}

void TEF6686::Get_Cmd(uint8_t mdl, uint8_t cmd, int16_t *receive, int len)
{
  uint8_t buf[3];
  buf[0] = mdl;
  buf[1] = cmd;
  buf[2] = 1;
  Write(buf, 3);
  Read((uint8_t *)receive, 2 * len);
  for (uint8_t i = 0; i < len; i++)
  {
    uint16_t newval = (uint8_t)(receive[i] >> 8) | (((uint8_t)(receive[i])) << 8);
    receive[i] = newval;
  }
}

void TEF6686::WriteInitData(const uint8_t *data)
{
  uint8_t *pa = (uint8_t *)data;
  uint8_t len, first;
  while (true)
  {
    len = pgm_read_byte_near(pa++);
    first = pgm_read_byte_near(pa);
    if (!len)
      break;
    if (len == 2 && first == 0xff)
    {
      int delaytime = pgm_read_byte_near(++pa);
      delay(delaytime);
      pa++;
    }
    else
    {
      Wire.beginTransmission(DEVICE_ADDR);
      for (int i = 0; i < len; i++)
        Wire.write(pgm_read_byte_near(pa++));
      Wire.endTransmission();
    }
  }
}

void TEF6686::Init()
{
  byte error;
  delay(40);
  Wire.begin(DEVICE_ADDR);
  uint16_t state[1];

  Wire.beginTransmission(DEVICE_ADDR);
  error = Wire.endTransmission();

  do
  {
    if (error != 0)
    {
      Serial.println(String("No I2C devices found"));
      return;
    }
    tefI2CComm.GetCommand(64, 128, state, 1);
    if (state[0] < 2)
    {
      Serial.println("Start writing init data");
      WriteInitData(INIT_DATA);
    }
    else if (state[0] >= 2)
    {
      Appl_Set_OperationMode(1);
      Serial.println("Init OK");
    }
  } while (state[0] < 2);
}

void TEF6686::Tune_To(uint8_t module, uint16_t freq)
{
  uint16_t params[2] = {1, freq};
  tefI2CComm.SetCommand(module, 1, params, 2);
  Currentfreq = freq;
  memset(rdsData.rtText, 0, sizeof(rdsData.rtText));
  memset(rdsData.psText, 0, sizeof(rdsData.psText));
}

void TEF6686::Audio_Set_Volume(uint8_t volume)
{
  uint16_t params[1] = {1};
  tefI2CComm.SetCommand(MODULE_AUDIO, 10, params, volume * 7 - 600);
}

void TEF6686::Audio_Set_Mute(uint8_t mute)
{
  uint16_t unmuteParams[1] = {mute};
  tefI2CComm.SetCommand(MODULE_AUDIO, 11, unmuteParams, 1);
}

void TEF6686::Appl_Set_OperationMode(uint8_t mode)
{
  uint16_t param[1] = {mode};
  tefI2CComm.SetCommand(MODULE_AUDIO, 1, param, 1);
}

void TEF6686::UpdateQualityStatus()
{
  uint16_t result[7];
  tefI2CComm.GetCommand(MODULE_FM, 128, result, 7);
  quality.AF_UpdateFlag = ((result[0] >> 15 & 1) == 1);
  quality.QualityTimeStamp = (uint16_t)(result[0] & 0x3ff);
  quality.Level = result[1];
  quality.Noise = result[2];
  quality.Wam = result[3];
  quality.Offset = result[4];
  quality.Bandwidth = result[5];
  quality.Modulation = result[6];
}

void TEF6686::UpdateRDSStatus()
{
  uint16_t uRds_Data[6] = {0};
  tefI2CComm.GetCommand(MODULE_FM, 130, uRds_Data, 7);

  if (((uRds_Data[0] >> 15) & 1) == 1)
  {
    HandleGroup0(uRds_Data);
    HandleGroup2(uRds_Data);
  }
}

void TEF6686::HandleGroup0(uint16_t *rdsRawData)
{
  uint8_t groupType = (uint8_t)((rdsRawData[2] >> 12) & 0xff);

  if (groupType != 0)
    return;

  rdsData.ms = (uint8_t)((rdsRawData[2] >> 3) & 1) == 1;
  rdsData.ta = (uint8_t)((rdsRawData[2] >> 4) & 1) == 1;
  rdsData.pty = (uint8_t)((rdsRawData[2] >> 5) & 0x1F);

  uint8_t err2 = (uint8_t)((rdsRawData[5] >> 12) & 3);
  uint8_t err4 = (uint8_t)((rdsRawData[5] >> 8) & 3);

  if (err2 == 0 && err4 == 0)
  {
    uint8_t offset = rdsRawData[2] & (uint16_t)3;
    rdsData.psText[2 * offset] = (uint8_t)(rdsRawData[4] >> 8);
    rdsData.psText[2 * offset + 1] = (uint8_t)(rdsRawData[4]);
  }
}

void TEF6686::HandleGroup2(uint16_t *rdsRawData)
{
  uint8_t groupType = (uint8_t)((rdsRawData[2] >> 12) & 0xff);

  if (groupType != 2)
    return;

  bool versionA = ((rdsRawData[0] >> 12) & 1) == 0;
  uint8_t offset = rdsRawData[2] & (uint16_t)0xf;

  uint8_t err2 = (uint8_t)((rdsRawData[5] >> 12) & 3);
  uint8_t err3 = (uint8_t)((rdsRawData[5] >> 10) & 3);
  uint8_t err4 = (uint8_t)((rdsRawData[5] >> 8) & 3);

  if (versionA)
  {
    if (err3 < 2)
    {
      rdsData.rtText[4 * offset] = (uint8_t)(rdsRawData[3] >> 8);
      rdsData.rtText[4 * offset + 1] = (uint8_t)(rdsRawData[3]);
    }
    if (err4 < 2)
    {
      rdsData.rtText[4 * offset + 2] = (uint8_t)(rdsRawData[4] >> 8);
      rdsData.rtText[4 * offset + 3] = (uint8_t)(rdsRawData[4]);
    }
  }
  else
  {
    if (err4 == 0)
    {
      rdsData.rtText[2 * offset] = (uint8_t)(rdsRawData[4] >> 8);
      rdsData.rtText[2 * offset + 1] = (uint8_t)(rdsRawData[4]);
    }
  }
}
