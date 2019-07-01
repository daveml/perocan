//#include "types.h"

namespace perocan 
{
using namespace perocan;

#ifdef HAL_CAN_USES_TEENSY


#include "libs/perocan/HAL_perocan.h"
#include <FlexCAN.h>

FlexCAN CAN(1000000);
static CAN_message_t msg,rxmsg;
CAN_filter_t CANfilter,CANFilterMask;

bool HAL_CAN::init() {
    return init(perocan::defaultDevType, perocan::defaultDevMfr, perocan::defaultDevId);
}

bool HAL_CAN::init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId)
{
  ApiCount=0;
  BufferedMsgCount=0;
  //
  // Setup filtering for just our messages
  //
  #define CAN_API_ALL 0x3FF
  // Use extended CAN
  CANFilterMask.ext = 1;
  // Return route disabled
  CANFilterMask.rtr = 0;
  // Mask is the inverted match on the CAN ID
  CANFilterMask.id = ~packCANId(DevType,DevMfr,CAN_API_ALL,DevId);

  // Ensure all the filters are zero'd
  for(int i=0; i<8; i++)
    CANbus.setFilter(CANfilter,i);
    
  CAN.begin(CANFilterMask);

  delay(500);
  Serial.println(F("Hello Teensy 3.2 FlexCAN Initialized"));
  sprintf(sbuf,"FlexCAN listening on CAN ID mask = %08lx\n", CANFilterMask.id);
  Serial.print(sbuf);

  return true;
}

bool HAL_CAN::send()
{

}

bool HAL_CAN::recv(HAL_CAL__Message_t *Msg, unit16_t Api)
{
  uint16_t rxapi;
  CAN_message_t rxmsg;
  
  /* Check if there is a new message */
  if(!CAN.read(rxmsg)) {
    /* If not, check to see if there is a buffered message pesding */
    rval = Api_find_Registered(Api);
    if(rval >= 0 && Buffer[rval].IsNew) {
       MsgCopy(Buffer[rval], RxMsg);
       Buffer[rva].IsNew = false;
       BufferedMsgCount++;
       return true;
    } else {
      return false;
    }
  }

  /* There is a message to read */
  rxapi = Api_getFrom_Id(rxmsg.id);
  /* Does it match what we are looking for? */
  if( rxapi == Api)
    // YES!
    MsgCopy(Msg, rxmsg);
    return true;
  else
  {
    // NO!, see if this an API we've registered to receive
    rval = Api_find_Registered(rxapi);
    if(rval >= 0) {
       // Yes, so buffer it. We only save the most recent.
       MsgCopy(Buffer[rval], RxMsg);
       Buffer[rva].IsNew = true;
       BufferedMsgCount++;
    }
    return false;
  }
  return false;
}

uint16_t HAL_CAN::Api_getFrom_Id(unit32_t Id)
{
  Id &= 0x0000FFFF;
  return (Id >> 6);
}

void HAL_CAN::MsgCopy(HAL_CAL__Message_t *Msg, CAN_message_t RxMsg);
  Msg.id = RxMsg.id;
  Msg.ext = RxMsg.ext;
  Msg.len = RxMsg.len;
  Msg.timeout = RxMsg.timeout;
  for(int idx=0; idx < RxMsg.len; idx++)
    Msg.buf[idx] = RxMsg.buf[i];
  Msg.api = Api_GetFrom_Id(RxMsg.id);  
}

bool HAL_CAN::available()
{
  return CAN.available() || (BufferedMsgCount>0) ;
}



#endif

#ifdef HAL_CAN_USES_ROBORIO
bool ::init()
{

}

bool ::send();
{

}

bool ::recv();
{

}



#endif

#ifdef HAL_CAN_USES_GENERICARDUINO
#include "libs/perocan/HAL_perocan.h"



#endif

}
