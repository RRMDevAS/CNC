#include "cnc_command.h"

#include <math.h>

NoneCommandData newNoneCommandData() {
    NoneCommandData data = {
        .miDummy = 0
    };
    return data;
}
MoveCommandData newMoveCommandData(float x, float y, float z) {
    MoveCommandData data = {
            .mfX = x,
            .mfY = y,
            .mfZ = z
        };
    return data;
}
PauseCommandData newPauseCommandData(float duration) {
    PauseCommandData data = {
        .mfDuration = duration
    };
    return data;
}
EndCommandData newEndCommandData() {
    EndCommandData data = {
        .miDummy = 0
    };
    return data;
}

SetSpeedCommandData newSetSpeedCommandData(float x, float y, float z) {
    SetSpeedCommandData data = {
            .mfSpeedX = x,
            .mfSpeedY = y,
            .mfSpeedZ = z
        };
    return data;
}

CncCommand newNoneCommand(uint32_t id) {
    NoneCommandData data = newNoneCommandData();
    union CommandData cmdData;
    cmdData.mNone = data;
    CncCommand cmd = {
        .muId = id,
        .mfElapsedTime = 0.0,
        .mfLength = 0.0,
        .meType = eNone,
        .mData = cmdData
    };

    return cmd;
}
CncCommand newMoveCommand(uint32_t id, float x, float y, float z) {
    MoveCommandData data = newMoveCommandData(x,y,z);
    union CommandData cmdData;
    cmdData.mMove = data;
    CncCommand cmd = {
        .muId = id,
        .mfElapsedTime = 0.0,
        .mfLength = 0.0,
        .meType = eMove,
        .mData = cmdData
    };

    return cmd;
}

CncCommand newPidParamsCommand(uint32_t id, PidParams *x, PidParams *y, PidParams *z) {
    PidParamsData data = {
        .maParams = {*x, *y, *z},
    };
    union CommandData cmdData;
    cmdData.mPidParams = data;
    CncCommand cmd = {
        .muId = id,
        .mfElapsedTime = 0.0,
        .mfLength = 0.0,
        .meType = ePidParamsUpdate,
        .mData = cmdData
    };

    return cmd;
}

CncCommand newPauseCommand(uint32_t id, float duration) {
    PauseCommandData data = newPauseCommandData(duration);
    union CommandData cmdData;
    cmdData.mPause = data;
    CncCommand cmd = {
        .muId = id,
        .mfElapsedTime = 0.0,
        .mfLength = 0.0,
        .meType = ePause,
        .mData = cmdData
    };

    return cmd;
}
CncCommand newEndCommand(uint32_t id) {
    EndCommandData data = newEndCommandData();
    union CommandData cmdData;
    cmdData.mEnd = data;
    CncCommand cmd = {
        .muId = id,
        .mfElapsedTime = 0.0,
        .mfLength = 0.0,
        .meType = eEnd,
        .mData = cmdData
    };

    return cmd;
}
CncCommand newSetSpeedCommand(uint32_t id, float x, float y, float z) {
    SetSpeedCommandData data = newSetSpeedCommandData(x,y,z);
    union CommandData cmdData;
    cmdData.mSetSpeed = data;
    CncCommand cmd = {
        .muId = id,
        .mfElapsedTime = 0.0,
        .mfLength = 0.0,
        .meType = eSetSpeed,
        .mData = cmdData
    };

    return cmd;
}

void updateNoneCommand(CncCommand *command, CncStatus *status, float delta) {
    
}
void updateMoveCommand(CncCommand *command, CncStatus *status, float delta) {
    status->maAxis[eX].mfTargetPosition = command->mData.mMove.mfX;
    status->maAxis[eY].mfTargetPosition = command->mData.mMove.mfY;
    status->maAxis[eZ].mfTargetPosition = command->mData.mMove.mfZ;
}

void updatePidParamsCommand(CncCommand *command, CncStatus *status, float delta) {
    status->maAxis[0].mSpeedControl.mParams = command->mData.mPidParams.maParams[0];
    status->maAxis[1].mSpeedControl.mParams = command->mData.mPidParams.maParams[1];
    status->maAxis[2].mSpeedControl.mParams = command->mData.mPidParams.maParams[2];

    status->maxTriggerUpdateMsg[ePidParams] = true;

    command->meType = eNone;
}

void updatePauseCommand(CncCommand *command, CncStatus *status, float delta) {

}
void updateEndCommand(CncCommand *command, CncStatus *status, float delta) {
    
}
void updateSetSpeedCommand(CncCommand *command, CncStatus *status, float delta) {
    status->maAxis[eX].mfTargetSpeed = command->mData.mSetSpeed.mfSpeedX;
    status->maAxis[eY].mfTargetSpeed = command->mData.mSetSpeed.mfSpeedY;
    status->maAxis[eZ].mfTargetSpeed = command->mData.mSetSpeed.mfSpeedZ;
}

void updateCommand(CncCommand *command, CncStatus *status, float delta) {
    command->mfElapsedTime += delta;
    switch (command->meType)
    {
    case eNone:
        updateNoneCommand(command, status, delta);
        break;
    case eMove:
        updateMoveCommand(command, status, delta);
        break;
    case ePidParamsUpdate:
        updatePidParamsCommand(command, status, delta);
        break;
    case ePause:
        updatePauseCommand(command, status, delta);
        break;
    case eEnd:
        updateEndCommand(command, status, delta);
        break;
    case eSetSpeed:
        updateSetSpeedCommand(command, status, delta);
        break;
    }
}

bool isNoneCommandDone(CncCommand *command, CncStatus *status) {
    return true;
}
bool isMoveCommandDone(CncCommand *command, CncStatus *status) {
    if (fabs(command->mData.mMove.mfX - status->maAxis[eX].mfPosition)>0.01) return false;
    if (fabs(command->mData.mMove.mfY - status->maAxis[eY].mfPosition)>0.01) return false;
    if (fabs(command->mData.mMove.mfZ - status->maAxis[eZ].mfPosition)>0.01) return false;
    return true;
}
bool isPauseCommandDone(CncCommand *command, CncStatus *status) {
    return command->mfElapsedTime >= command->mfLength; 
}
bool isEndCommandDone(CncCommand *command, CncStatus *status) {
    return true;
}
bool isSetSpeedCommandDone(CncCommand *command, CncStatus *status) {
    if (fabs(command->mData.mSetSpeed.mfSpeedX - status->maAxis[eX].mfSpeed)>0.0001) return false;
    if (fabs(command->mData.mSetSpeed.mfSpeedY - status->maAxis[eY].mfSpeed)>0.0001) return false;
    if (fabs(command->mData.mSetSpeed.mfSpeedZ - status->maAxis[eZ].mfSpeed)>0.0001) return false;
    return true;
}

bool isCommandDone(CncCommand *command, CncStatus *status) {
    switch (command->meType)
    {
    case eNone:
        return isNoneCommandDone(command, status);
        break;
    case eMove:
        return isMoveCommandDone(command, status);
        break;
    case ePidParamsUpdate:
        return true;
        break;
    case ePause:
        return isPauseCommandDone(command, status);
        break;
    case eEnd:
        return isEndCommandDone(command, status);
        break;
    case eSetSpeed:
        return isSetSpeedCommandDone(command, status);
        break;
    }

    return true;
}