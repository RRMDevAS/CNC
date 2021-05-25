#ifndef CNC_COMMAND_H
#define CNC_COMMAND_H

#include <stdint.h>

#include "cnc_status.h"

struct NoneCommandData{
    int32_t miDummy;
};
typedef struct NoneCommandData NoneCommandData;

struct MoveCommandData
{
    float mfX, mfY, mfZ;
};
typedef struct MoveCommandData MoveCommandData;

struct PauseCommandData{
    float mfDuration;
};
typedef struct PauseCommandData PauseCommandData;

struct EndCommandData{
    int32_t miDummy;
};
typedef struct EndCommandData EndCommandData;

struct CncCommand
{
    uint32_t muId;

    float mfElapsedTime, mfLength;

    enum Type{
        eNone = 0,
        eMove,
        ePause,
        eEnd,
    } meType;

    union CommandData
    {
        NoneCommandData mNone;
        MoveCommandData mMove;
        PauseCommandData mPause;
        EndCommandData mEnd;
    } mData;
};
typedef struct CncCommand CncCommand;

NoneCommandData newNoneCommandData();
MoveCommandData newMoveCommandData(float x, float y, float z);
PauseCommandData newPauseCommandData(float duration);
EndCommandData newEndCommandData();

CncCommand newNoneCommand(uint32_t id);
CncCommand newMoveCommand(uint32_t id, float x, float y, float z);
CncCommand newPauseCommand(uint32_t id, float duration);
CncCommand newEndCommand(uint32_t id);

void updateNoneCommand(CncCommand *command, CncStatus *status, float delta);
void updateMoveCommand(CncCommand *command, CncStatus *status, float delta);
void updatePauseCommand(CncCommand *command, CncStatus *status, float delta);
void updateEndCommand(CncCommand *command, CncStatus *status, float delta);

void updateCommand(CncCommand *command, CncStatus *status, float delta);

bool isNoneCommandDone(CncCommand *command, CncStatus *status);
bool isMoveCommandDone(CncCommand *command, CncStatus *status);
bool isPauseCommandDone(CncCommand *command, CncStatus *status);
bool isEndCommandDone(CncCommand *command, CncStatus *status);

bool isCommandDone(CncCommand *command, CncStatus *status);

#endif          // CNC_COMMAND_H