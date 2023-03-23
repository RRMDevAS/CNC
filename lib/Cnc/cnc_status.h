#ifndef CNC_STATUS_H
#define CNC_STATUS_H

#include "axis.h"

enum EStatusMsgType{
    eCoordinates = 0,
    eStatus,
    ePidParams,
    eStatusCount
};
typedef enum EStatusMsgType EStatusMsgType;

struct CncStatus {
    int32_t miCycleTime, miStatusUpdate;
    Axis maAxis[eAxisCount];
    bool maxTriggerUpdateMsg[eStatusCount];
};
typedef struct CncStatus CncStatus;

struct AxisStatusMsg {
    float mfPosition;
    float mfSpeed;
    float mfTargetPosition;
    float mfTargetSpeed;
    float mfPidPropControl, mfPidIntControl, mfPidDerControl;
    int32_t miDuty;
};
typedef struct AxisStatusMsg AxisStatusMsg;

struct CncStatusMsgData{
    int32_t miCycleTime;
    AxisStatusMsg maAxis[eAxisCount];
};
typedef struct CncStatusMsgData CncStatusMsgData;

struct CncCoordsMsgData{
    float mafX[eAxisCount];
};
typedef struct CncCoordsMsgData CncCoordsMsgData;

struct CncPidParamsMsgData{
    PidParams maParams[eAxisCount];
};
typedef struct CncPidParamsMsgData CncPidParamsMsgData;

struct CncStatusMsg {
    EStatusMsgType meType;
    union {
        CncCoordsMsgData coords;
        CncStatusMsgData status;
        CncPidParamsMsgData params;
    };
};
typedef struct CncStatusMsg CncStatusMsg;

CncStatusMsg generateStatusMsg(const CncStatus *status, EStatusMsgType msgType);

bool getStatusMessage(CncStatus *cnc, CncStatusMsg* msg);

void updateStatus(CncStatus *cnc, int32_t iMSecs);

#endif      // CNC_STATUS_H