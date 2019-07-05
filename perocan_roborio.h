/*
  perocan
  perocan_roborio.h
*/
#ifndef PEROCAN_ROBORIO_H
#define PEROCAN_ROBORIO_H

#include "perocan.h"
#ifdef PEROCAN_USE_ROBORIO

#include "Robot.h"
#include <frc/CAN.h>

namespace perocan
{
	
class perocan_roborio : public perocan__base
{
public:
  bool init(uint8_t DevType, uint8_t DevMfr, uint8_t DevId);
  bool init() {
	  return init(perocan::defaultDevType, perocan::defaultDevMfr, perocan::defaultDevId); }
  bool send(uint8_t *Data, int Len, int Api);
  bool recv(perocan_message_t *Msg, uint16_t Api);
  
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
#endif
#endif
