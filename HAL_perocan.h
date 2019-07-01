/*
  perocan
  HAL_perocan.h
*/

#ifndef HAL_PEROCAN_H
#define HAL_PEROCAN_H

//#define HAL_CAN_USES_TEENSY
#define HAL_CAN_USES_ROBORIO
//#define HAL_CAN_USES_GENERICARDUINO

namespace perocan
{
  
class HAL_CAN
{
public:

  bool init();
  bool send();
  bool recv();

 private:
  unsigned get_CANid();
    
};

class HAL_CAN__teensy
{
 public:

};

class HAL_CAN__roborio
{
 public:

};

class HAL_CAN__genericArduino
{
 public:

};

}
#endif
