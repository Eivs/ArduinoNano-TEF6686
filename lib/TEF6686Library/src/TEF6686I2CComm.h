#ifndef TEF6686I2CCOMM_H
#define TEF6686I2CCOMM_H

#include <Arduino.h>

class TEF6686I2CComm
{
private:
  uint8_t DEVICE_ADDR = 0x65;

public:
  void Write(uint8_t *data, uint8_t length);
  void Read(uint8_t *data, uint8_t length);
  void Set_Cmd(uint8_t mdl, uint8_t cmd, int len, ...);
  void Get_Cmd(uint8_t mdl, uint8_t cmd, int16_t *receive, int len);
  void dsp_write_data(const uint8_t *data);
  void GetCommand(uint8_t module, uint8_t cmd, uint16_t *response, uint8_t responseLength);
  void SetCommand(uint8_t module, uint8_t cmd, uint16_t *params, uint8_t paramsCount);
};
#endif
