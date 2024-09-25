#include "TEF6686I2CComm.h"
#include <Wire.h>

void TEF6686I2CComm::Write(uint8_t *buf, uint8_t len)
{
  Wire.beginTransmission(DEVICE_ADDR);
  for (int i = 0; i < len; i++)
    Wire.write(*buf++);
  Wire.endTransmission();
}

void TEF6686I2CComm::Read(uint8_t *buf, uint8_t len)
{
  uint8_t lenrec = Wire.requestFrom(DEVICE_ADDR, len);
  for (int i = 0; i < lenrec; i++)
    *buf++ = Wire.read();
}

void TEF6686I2CComm::Set_Cmd(uint8_t mdl, uint8_t cmd, int len, ...)
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

void TEF6686I2CComm::Get_Cmd(uint8_t mdl, uint8_t cmd, int16_t *receive, int len)
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

void TEF6686I2CComm::GetCommand(uint8_t module, uint8_t cmd, uint16_t *response, uint8_t responseLength)
{
  Wire.beginTransmission(DEVICE_ADDR);
  Wire.write(module);
  Wire.write(cmd);
  Wire.write(1);
  Wire.endTransmission();
  uint8_t dataLength = Wire.requestFrom(DEVICE_ADDR, (uint8_t)(responseLength * 2));
  for (int i = 0; i < dataLength / 2; i++)
  {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    response[i] = msb << 8 | lsb;
  }
}

void TEF6686I2CComm::SetCommand(uint8_t module, uint8_t cmd, uint16_t *params, uint8_t paramsCount)
{
  Wire.beginTransmission(DEVICE_ADDR);
  Wire.write(module);
  Wire.write(cmd);
  Wire.write(1);
  for (int i = 0; i < paramsCount; i++)
  {
    uint8_t msb = params[i] >> 8;
    uint8_t lsb = params[i];
    Wire.write(msb);
    Wire.write(lsb);
  }
  Wire.endTransmission();
}
