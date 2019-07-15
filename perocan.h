/*
  perocan
  HAL_perocan.h
*/
#ifndef HAL_PEROCAN_H
#define HAL_PEROCAN_H

#include "perocan_types.h"
#include "perocan_messages.h"

namespace perocan
{

typedef struct {
  unsigned char manufacturer;
  unsigned char deviceType;  
  unsigned char deviceId;
} CANStorage_t;


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


#ifdef PEROCAN_USE_ARDUINO

#include <Arduino.h>
#include <FlexCAN.h>

namespace perocan
{
FlexCAN CAN(1000000);
//static CAN_message_t msg;
CAN_filter_t CANfilter,CANFilterMask;
char sbuf[10];
	
class perocan_arduino : public perocan__base
{
public:
  bool init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId){
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
  bool init() {
	return init(perocan::defaultDevType, perocan::defaultDevMfr, perocan::defaultDevId); }
  bool send(uint8_t *Data, int Len, int Api){
	CAN_message_t txmsg;
	txmsg.id = Id_getFrom_Api(Api);
	txmsg.ext = 1;
	txmsg.len = Len;
	txmsg.timeout = 0;
	for(int idx=0; idx < Len; idx++)
		txmsg.buf[idx] = Data[idx];
	int rval = CAN.write(txmsg);
	return rval;
  }
  bool recv(perocan_message_t *Msg, uint16_t Api)
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

  bool available();
  
private:
  void copy_rxMsg(perocan_message_t *Msg, CAN_message_t RxMsg){
	Msg->id = RxMsg.id;
	Msg->ext = RxMsg.ext;
	Msg->len = RxMsg.len;
	Msg->timeout = RxMsg.timeout;
	for(int idx=0; idx < RxMsg.len; idx++)
		Msg->data[idx] = RxMsg.buf[idx];
	Msg->api = Api_getFrom_Id(RxMsg.id);  
  }
  void copy_txMsg(CAN_message_t *TxMsg, perocan_message_t *Msg)
  {
	TxMsg->id = Msg->id;
	TxMsg->ext = Msg->ext;
	TxMsg->len = Msg->len;
	TxMsg->timeout = Msg->timeout;
	for(int idx=0; idx < Msg->len; idx++)
		TxMsg->buf[idx] = Msg->data[idx];
  }
};

}
#endif //#ifdef PEROCAN_USE_ARDUINO



#ifdef PEROCAN_USE_ROBORIO

#include "Robot.h"
#include <frc/CAN.h>

namespace perocan
{
	
class perocan_roborio : public perocan__base
{
public:
/*
  bool init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId);
  bool init();
	bool send(uint8_t *Data, int Len, int Api);
  bool recv(perocan_message_t *Msg, uint16_t Api);
*/

  bool init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId){
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
  bool init() {
	  return init(perocan::defaultDevType, perocan::defaultDevMfr, perocan::defaultDevId); }

	bool send(uint8_t *Data, int Len, int Api){
		if(!checkHandle())
			return false;
		CANp->WritePacket(Data, Len, Api);
		return true;
	}

  bool recv(perocan_message_t *Msg, uint16_t Api){
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

private:
  bool checkHandle() {
    if(CANp == 0) {
      printf("perocan: Invalid CANp handle\n");
      return false;
    }
    return true;
  }
  frc::CAN *CANp=0;
};

}
#endif // #ifdef PEROCAN_USE_ROBORIO



#ifdef PEROCAN_USE_GENSPI

#endif // #ifdef PEROCAN_USE_GENSPI

#endif //#ifndef HAL_PEROCAN_H


