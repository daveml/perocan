#ifdef PEROCAN_USE_ROBORIO

#include "Robot.h"
#include "perocan_roborio.h"
#include <string>

#include "perocan_types.h"

namespace perocan 
{
using namespace perocan;

bool perocan_roborio::init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId)
{
  ApiCount=0;
  BufferedMsgCount=0;

  CANp = new frc::CAN(DevId,DevMfr,DevType);

  if(!checkHandle()) {
    printf("Hello, perocan_roborio Initialization FAILED\n");
    return false;
  }

  printf("Hello, perocan_roborio Initialized DT=%d DM=%d DI=%d\n",DevType,DevMfr,DevId);
  return true;
}

bool perocan_roborio::send(uint8_t *Data, int Len, int Api)
{
  if(!checkHandle())
    return false;
  CANp->WritePacket(Data, Len, Api);
  return true;
}

bool perocan_roborio::recv(perocan_message_t *Msg, uint16_t Api)
{
  if(!checkHandle())
    return false;

  frc::CANData RxCANData;
  if( CANp->ReadPacketNew(Api, &RxCANData) ) {
    Msg->len = RxCANData.length;
    std::memcpy(Msg->data, RxCANData.data, RxCANData.length);
    return true;
  }

  return false;
}

} //namespace

#endif