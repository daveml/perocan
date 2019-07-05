#include <perocan.h>

namespace perocan 
{
using namespace perocan;




}

#ifdef PEROCAN_USE_ARDUINO_POOP

#include <perocan_arduino.h>
#include <FlexCAN.h>
#include "perocan_types.h"
namespace perocan 
{
using namespace perocan;


bool perocan_arduino::init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId)


bool perocan_arduino::send(uint8_t *Data, int Len, int Api)


bool perocan_arduino::recv(perocan_message_t *Msg, uint16_t Api)
{
  uint16_t rxapi;
  CAN_message_t rxmsg;
  
  /* Check if there is a new message */
  if(!CAN.available()) {
  /* If not, check to see if there is a buffered message pesding */
    int rval = Api_find_Registered(Api);
    if(rval >= 0 && Buffer[rval].IsNew) {
       copy_rxMsg(&Buffer[rval], rxmsg);
       Buffer[rval].IsNew = false;
       BufferedMsgCount++;
       return true;
    } else {
      return false;
    }
  }
  if(!CAN.read(rxmsg)) 
	  return false;
	
  /* There is a message to read */
  rxapi = Api_getFrom_Id(rxmsg.id);
  /* Does it match what we are looking for? */
  if( rxapi == Api) {
	// YES!
    copy_rxMsg(Msg, rxmsg);
    return true;
  }
  else
  {
    int rval = Api_find_Registered(rxapi);
    if(rval >= 0) {
       // Yes, so buffer it. We only save the most recent.
       copy_rxMsg(&Buffer[rval], rxmsg);
       Buffer[rval].IsNew = true;
       BufferedMsgCount++;
    }
    return false;
  }
  return false;
}

void perocan_arduino::copy_rxMsg(perocan_message_t *Msg, CAN_message_t RxMsg){
  Msg->id = RxMsg.id;
  Msg->ext = RxMsg.ext;
  Msg->len = RxMsg.len;
  Msg->timeout = RxMsg.timeout;
  for(int idx=0; idx < RxMsg.len; idx++)
    Msg->buf[idx] = RxMsg.buf[idx];
  Msg->api = Api_getFrom_Id(RxMsg.id);  
}

void perocan_arduino::copy_txMsg(CAN_message_t *TxMsg, perocan_message_t *Msg){
  TxMsg->id = Msg->id;
  TxMsg->ext = Msg->ext;
  TxMsg->len = Msg->len;
  TxMsg->timeout = Msg->timeout;
  for(int idx=0; idx < Msg->len; idx++)
    TxMsg->buf[idx] = Msg->buf[idx];
}

bool perocan_arduino::available()
{
  bool avail = CAN.available();
  bool count = (BufferedMsgCount>0);
//  sprintf(sbuf,"available: avail=%d  count=%d\n", avail, count);
//  Serial.print(sbuf);
  
  return (avail || count);
}

} //namespace

#endif

#ifdef PEROCAN_USE_ROBORIO_POOP

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

#ifdef PEROCAN_USE_GENSPI_POOP

#endif


