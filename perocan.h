/*
  perocan
  HAL_perocan.h
*/
#ifndef HAL_PEROCAN_H
#define HAL_PEROCAN_H

#include "perocan_types.h"

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
  uint8_t data[8];
  bool IsNew;
} perocan_message_t;

const uint8_t defaultDevType = 0x0F;
const uint8_t defaultDevMfr  = 0x03;
const uint8_t defaultDevId   = 0x01;

class perocan__base
{
public:

  bool init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId){ return false;}
  
  void register_API(uint16_t Api) {
    Buffer[ApiCount].IsNew = false;
    Buffer[ApiCount].api = Api; 
    ApiCount++;
  }
  
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

  uint16_t ApiCount;
  perocan_message_t Buffer[8];
  uint16_t BufferedMsgCount;
  uint32_t CANApiMask;
};

}
#endif
