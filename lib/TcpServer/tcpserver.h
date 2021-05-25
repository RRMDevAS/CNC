#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define TCPSERVERERROR_SOCKET       1
#define TCPSERVERERROR_BIND         2
#define TCPSERVERERROR_LISTEN       4
#define TCPSERVERERROR_ACCEPT       8
#define TCPSERVERERROR_RECEIV       16
#define TCPSERVERERROR_SEND         32

enum EServerState {
    eNew=0,
    eInitialized,
    eSocketCreated,
    eSocketBound,
    eListening,
    eConnected,
    eReceiving,
    eSending,
    eTimeout
};
typedef enum EServerState EServerState;

struct DataBuffer{
    uint32_t muSize;
    uint8_t mabData[512];
};
typedef struct DataBuffer DataBuffer;

struct TcpClientInfo {
    int miSocket;
    struct sockaddr_in mSockAddr;
    DataBuffer mReceiveBuffer, mSendBuffer;
};
typedef struct TcpClientInfo TcpClientInfo;

struct TcpServer
{
    int miListenSocket;
    struct sockaddr_in mSockAddr;
    EServerState meState;

    int64_t miErrorFlags;

    TcpClientInfo mClient;
};
typedef struct TcpServer TcpServer;

TcpServer newTcpServer(const char* ipAddress, int16_t port);

void initializeTcpServer(TcpServer *pTcpServer);
bool acceptConnection(TcpServer *pTcpServer);
bool receiveData(TcpServer *pTcpServer);
bool sendData(TcpServer *pTcpServer);
void disconnectAll(TcpServer *pTcpServer);
void shutdownServer(TcpServer *pTcpServer);
void printTcpStatus(TcpServer *pTcpServer);

void pushToBuffer(DataBuffer *pBuffer, uint32_t byteCount, uint8_t *data);
void pullFromBuffer(DataBuffer *pBuffer, uint32_t *byteCount, uint8_t *destination);

#endif // TCPSERVER_H