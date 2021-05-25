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

void updateNoneCommand(CncCommand *command, CncStatus *status, float delta) {
    
}
void updateMoveCommand(CncCommand *command, CncStatus *status, float delta) {
    status->maAxis[eX].mfTargetPosition = command->mData.mMove.mfX;
    status->maAxis[eY].mfTargetPosition = command->mData.mMove.mfY;
    status->maAxis[eZ].mfTargetPosition = command->mData.mMove.mfZ;
}
void updatePauseCommand(CncCommand *command, CncStatus *status, float delta) {

}
void updateEndCommand(CncCommand *command, CncStatus *status, float delta) {
    
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
    case ePause:
        updatePauseCommand(command, status, delta);
        break;
    case eEnd:
        updateEndCommand(command, status, delta);
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

bool isCommandDone(CncCommand *command, CncStatus *status) {
    switch (command->meType)
    {
    case eNone:
        return isNoneCommandDone(command, status);
        break;
    case eMove:
        return isMoveCommandDone(command, status);
        break;
    case ePause:
        return isPauseCommandDone(command, status);
        break;
    case eEnd:
        return isEndCommandDone(command, status);
        break;
    }

    return true;
}