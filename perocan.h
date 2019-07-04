/*
  perocan
  HAL_perocan.h
*/
#ifndef HAL_PEROCAN_H
#define HAL_PEROCAN_H

#include <Arduino.h>

#define HAL_CAN_USES_TEENSY
//#define HAL_CAN_USES_ROBORIO
//#define HAL_CAN_USES_GENERICARDUINO

#include <FlexCAN.h>

namespace perocan
{
typedef struct {
  unsigned char manufacturer;
  unsigned char deviceType;  
  unsigned char deviceId;
} CANStorage_t;

typedef struct {
  uint16_t api;
  uint8_t ext; // identifier is extended
  uint8_t len; // length of data
  uint32_t id; // can identifier
  uint16_t timeout; // milliseconds, zero will disable waiting
  uint8_t buf[8];
  bool IsNew;
} HAL_CAN__Message_t;

const uint8_t defaultDevType = 0x0F;
const uint8_t defaultDevMfr  = 0x03;
const uint8_t defaultDevId   = 0x01;

class HAL_CAN
{
public:

  bool init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId);
  bool init() ;
  bool send(uint8_t *Data, int Len, int Api);
  bool recv(HAL_CAN__Message_t *Msg, uint16_t Api);
  bool available();
  
  void register_API(uint16_t Api) {
    Buffer[ApiCount].IsNew = false;
    Buffer[ApiCount].api = Api; 
    ApiCount++;
  }
  
 private:
  uint32_t CreateCANId(CANStorage_t* storage, unsigned apiId) {
      return packCANId(storage->deviceType, storage->manufacturer, apiId, storage->deviceId);
  }
  uint16_t Api_getFrom_Id(uint32_t Id) {
    Id &= 0x0000FFFF;
    return (Id >> 6);
  }
  uint32_t Id_getFrom_Api(uint16_t Api) {
	  return CANApiMask | (Api << 6);
  }
  void copy_rxMsg(HAL_CAN__Message_t *Msg, CAN_message_t RxMsg);
  void copy_txMsg(CAN_message_t *TxMsg, HAL_CAN__Message_t *Msg);

  int Api_find_Registered(uint16_t Api) {
    for(int idx=0; idx<ApiCount; idx++)
    {
      if(Buffer[idx].api == Api)
        return idx;
    }
    return -1;
  }

  uint32_t packCANId(unsigned devType, unsigned devMfr, unsigned apiId, unsigned devId) {
    uint32_t id = 0;
    id |= (devType & 0x1F) << 24;
    id |= (devMfr & 0xFF) << 16;
    id |= (apiId & 0x3FF) << 6;
    id |= (devId & 0x3F);
    return id;
  }

  CANStorage_t MyDev;
  uint16_t ApiCount;
  HAL_CAN__Message_t Buffer[8];
  uint16_t BufferedMsgCount;
  uint32_t CANApiMask;
};

}
#endif
