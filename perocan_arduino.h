/*
  perocan
  HAL_perocan.h
*/
#ifndef PEROCAN_ARDUINO_H
#define PEROCAN_ARDUINO_H

#ifdef PEROCAN_USE_ARDUINO

#include "perocan.h"
#include <Arduino.h>
#include <FlexCAN.h>

namespace perocan
{
	
class perocan_arduino : public perocan__base
{
public:
  bool init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId);
  bool init() {
	return init(perocan::defaultDevType, perocan::defaultDevMfr, perocan::defaultDevId); }
  bool send(uint8_t *Data, int Len, int Api);
  bool recv(perocan_message_t *Msg, uint16_t Api);
  bool available();
  
private:
  void copy_rxMsg(perocan_message_t *Msg, CAN_message_t RxMsg);
  void copy_txMsg(CAN_message_t *TxMsg, perocan_message_t *Msg);
};

}
#endif
#endif
