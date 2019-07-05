#include <perocan_arduino.h>
#include <FlexCAN.h>
#include "perocan_types.h"
namespace perocan 
{
using namespace perocan;

FlexCAN CAN(1000000);
//static CAN_message_t msg;
CAN_filter_t CANfilter,CANFilterMask;
char sbuf[10];

bool perocan_arduino::init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId)
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

  CANApiMask = packCANId(DevType,DevMfr,0x000,DevId);
  
  CANfilter.rtr = 0;
  CANfilter.ext = 1;
  CANfilter.id = 0;  
  // Ensure all the filters are zero'd
  for(int i=0; i<8; i++)
    CAN.setFilter(CANfilter,i);
    
  CAN.begin(CANFilterMask);

  delay(500);
  Serial.println(F("Hello Teensy 3.2 FlexCAN Initialized"));
  sprintf(sbuf,"FlexCAN listening on CAN ID mask = %08lx\n", ~CANFilterMask.id);
  Serial.print(sbuf);

  return true;
}

bool perocan_arduino::send(uint8_t *Data, int Len, int Api)
{
  CAN_message_t txmsg;
  txmsg.id = Id_getFrom_Api(Api);
  txmsg.ext = 1;
  txmsg.len = Len;
  txmsg.timeout = 0;
  for(int idx=0; idx < Len; idx++)
    txmsg.buf[idx] = Data[idx];
//  sprintf(sbuf,"Sent API=%08lX Ext=%d Len=%d to=%d\n", txmsg.id,txmsg.ext,txmsg.len,txmsg.timeout);
//  Serial.print(sbuf);
  int rval = CAN.write(txmsg);
/*   if(!rval)
	  Serial.println("Bad exit status from CAN.write!");
  else
	  Serial.println("Tx OKay");
  sprintf(sbuf,"Sent API=%08lX Ext=%d Len=%d to=%d rval=%d\n", txmsg.id,txmsg.ext,txmsg.len,txmsg.timeout,rval);
  Serial.print(sbuf);
 */  
  return rval;
}

bool perocan_arduino::recv(perocan_message_t *Msg, uint16_t Api)
{
  uint16_t rxapi;
  CAN_message_t rxmsg;
  
  /* Check if there is a new message */
  if(!CAN.available()) {
//	sprintf(sbuf, "read called w/o available, checking buffers: count=%d\n", BufferedMsgCount);
//	Serial.print(sbuf);
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
//	Serial.println("RX API Match");
	// YES!
    copy_rxMsg(Msg, rxmsg);
    return true;
  }
  else
  {
	//Serial.println("RX API DID NOT Match");
    // NO!, see if this an API we've registered to receive
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
