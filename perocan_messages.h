#ifndef PEROCAN_MESSAGES_H
#define PEROCAN_MESSAGES_H

#include "perocan_types.h"

namespace perocan {


typedef struct {
  uint16_t api;
  uint8_t ext; // identifier is extended
  uint8_t len; // length of data
  uint32_t id; // can identifier
  uint16_t timeout; // milliseconds, zero will disable waiting
  uint8_t data[8];
  bool IsNew;
} perocan_message_t;

typedef enum {
	API_REQ = 0,
	API_RSP = 1,
	API_IND = 2
} msg_api_e;

typedef enum {
	STATUS_REQ = 1,
	STATUS_RSP,
	CONFIG_REQ,
	CONFIG_RSP,
	INPUT_REQ,
	INPUT_RSP,
	INPUT_IND,
	PWMSET_REQ,
	PWMSET_RSP,
	DIOSET_REQ,
	DIOSET_RSP
} msg_cmd_e;

typedef struct {
	msg_api_e ApiID;
	msg_cmd_e Cmd;
} msg_t;

const msg_t C2H_PM_STATUS_REQ_msg = {API_REQ, STATUS_REQ};
const msg_t H2C_PM_STATUS_RSP_msg = {API_RSP, STATUS_RSP};
const msg_t C2H_PM_CONFIG_REQ_msg = {API_REQ, CONFIG_REQ};
const msg_t H2C_PM_CONFIG_RSP_msg = {API_RSP, CONFIG_RSP};
const msg_t C2H_PM_INPUT_REQ_msg = {API_REQ, INPUT_REQ};
const msg_t H2C_PM_INPUT_RSP_msg = {API_RSP, INPUT_RSP};
const msg_t H2C_PM_INPUT_IND_msg = {API_IND, INPUT_IND}; // *
const msg_t C2H_PM_PWMSET_REQ_msg = {API_REQ, PWMSET_REQ};
const msg_t H2C_PM_PWMSET_RSP_msg = {API_RSP, PWMSET_RSP};
const msg_t C2H_PM_DIOSET_REQ_msg = {API_REQ, DIOSET_REQ};
const msg_t H2C_PM_DIOSET_RSP_msg = {API_RSP, DIOSET_RSP};

const msg_t ReqMsgs[5] = {
	{API_REQ, STATUS_REQ},
    {API_REQ, CONFIG_REQ},
    {API_REQ, INPUT_REQ},
    {API_REQ, PWMSET_REQ},
    {API_REQ, DIOSET_REQ},
	};

const msg_t RspMsgs[6] = {
	{API_RSP, STATUS_RSP},
    {API_RSP, CONFIG_RSP},
    {API_RSP, INPUT_RSP},
    {API_RSP, PWMSET_RSP},
	{API_RSP, DIOSET_RSP}
	};

const msg_t IndMsgs[1] = {
		{API_IND, INPUT_IND}
	};

static msg_cmd_e Cmd_getFrom_Data(perocan_message_t *Msg) {
	return (msg_cmd_e)Msg->data[0];
}

class msg_base
{
public:
	void txInit(msg_t Msg, bool Rtr=0){
		ApiID = Msg.ApiID;
		Data[0] = Msg.Cmd | 0x7F;
        /* If ReTurn Response requested, set the MSB */
        /* NOTE this is not CAN RTR */
        if(Rtr)
            Data[0] |= 0x80;
	}

	void rxInit(perocan_message_t *CANData){
        setData_Raw(CANData->data);
	}

    bool isSet_Rtr()
    {
        return (Data[0] & 0x80);
    }

	void setData_Raw(uint8_t *DataIn) {
		for(int i=0; i<8; i++)
			Data[i] = DataIn[i];
	}

    /*bool SetRespData(perocan_message_t *CANData) {
        switch(Cmd_getFrom_Data(CANData)) {
            case STATUS_REQ:
                //Data[0]
	        case CONFIG_REQ:
	        case INPUT_REQ:
	        case PWMSET_REQ:
	        case DIOSET_REQ:


                
        }
    }*/

	msg_api_e ApiID;
	uint8_t Data[8];
};

class H2C_PM_INPUT_IND : public msg_base {
public:
    void txInit() {
        msg_base::txInit(H2C_PM_INPUT_IND_msg);
    }

	void fill(uint16_t A3, uint16_t A2,uint16_t A1,uint16_t A0, uint8_t D5, uint8_t D4, uint8_t D3, uint8_t D2, uint8_t D1, uint8_t D0) {
		Data[1] = 0;
		Data[2] = A3 >> 2;
		Data[3] = (A3 << 6) | (A2 >> 4);
		Data[4] = ((A2 & 0x0F) << 4) | (A1 >> 6);
		Data[5] = ((A1 & 0x3F) << 2) | (A0 >> 8);
		Data[6] = (A0 & 0xFF);
		Data[7] = ((D5 & 0x1) << 5) | ((D4 & 0x1) << 4) | ((D3 & 0x1) << 3) | ((D2 & 0x1) << 2) | ((D1 & 0x1) << 1) | (D0 & 0x1);
	}
	void parse() {
		A[0] = ((Data[5] & 0x03) << 8) | (Data[6]);
		A[1] = ((Data[4] & 0x0F) << 6) | (Data[5] >> 2);
		A[2] = ((Data[3] & 0x3F) << 4) | (Data[4] >> 4);
		A[3] = ((Data[2] & 0xFF) << 2) | (Data[3] >> 6);
		uint8_t tmp = Data[7];
		for(int i=0; i<6; i++) {
			D[i] = tmp & 0x01;
			tmp >>= 1;
		}
	}

	uint16_t A[4];
	bool D[6];
};

class C2H_PM_DIOSET_REQ : public msg_base {
    void txInit() {
        txInit(false);
    }

    void txInit(bool Rtr) {
        msg_base::txInit(C2H_PM_DIOSET_REQ_msg, Rtr);
    }

    /* B1   | B2   | B3   | B4   | B5   | B6   | B7  */
    /* Op   |  Dir | */
    /* Where Op=0 -> no operation, Op=1 -> Set direction */
    /* Where Dir=0 -> Input, Dir=1 -> Output */
    void fill(uint8_t Op, uint8_t Dir) {
        Data[1] = Op;
        Data[2] = Dir;
    }
    
    void parse() {
        Op=Data[1];
        Dir=Data[2];
    }
    uint8_t Op;
    uint8_t Dir;
};

} // namespace perocan


#endif