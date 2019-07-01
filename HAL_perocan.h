/*
  perocan
  HAL_perocan.h
*/
#ifndef HAL_PEROCAN_H
#define HAL_PEROCAN_H

#include "types.h"

//#define HAL_CAN_USES_TEENSY
#define HAL_CAN_USES_ROBORIO
//#define HAL_CAN_USES_GENERICARDUINO


namespace perocan
{
typedef struct CANStorage {
  unsigned char manufacturer;
  unsigned char deviceType;
  unsigned char deviceId;
} CANStorage_t;

typedef struct HAL_CAN__Message_t {
  uint16_t api;
  uint8_t ext; // identifier is extended
  uint8_t len; // length of data
  uint32_t id; // can identifier
  uint16_t timeout; // milliseconds, zero will disable waiting
  uint8_t buf[8];
  boot IsNew;
}

class HAL_CAN
{
public:
  const uint8_t defaultDevType = 0x0F;
  const uint8_t defaultDevMfr  = 0x03;
  const uint8_t defaultDevId   = 0x01;


  bool init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId);
  bool init() ;
  bool send();
  bool recv();
  bool available();
  void register_API(uint16_t Api) {
    Buffer[ApiCount].isNew = false;
    Buffer[ApiCount].api = Api; 
    ApiCount++;
  }
 private:
  uint32_t CreateCANId(CANStorage_t* storage, unsigned apiId) {
      return packCANId(storage->deviceType, storage->manufacturer, apiId, storage->deviceId);
  }

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
};

}
#endif
