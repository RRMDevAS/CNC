#ifndef COMMAND_SERDE_H
#define COMMAND_SERDE_H

#include "cnc_command.h"
#include "cnc_status.h"

#include <stdint.h>

int32_t deserializeCommand(CncCommand *cmd, int32_t byteCount, uint8_t *buffer);

int32_t serializeStatus(const CncStatusMsg* status, int32_t *byteCount, uint8_t *buffer);

#endif      // COMMAND_SERDE_H