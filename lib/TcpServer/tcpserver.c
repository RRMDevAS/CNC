#include "tcpserver.h"

#include "esp_log.h"

#include "fcntl.h"

#define PORT 250

#define TAG "TCP_SERVER"

TcpServer newTcpServer(const char* ipAddress, int16_t port) {
    TcpServer tcp;

    ESP_LOGI(TAG, "new for %s:%d", ipAddress, port);

    tcp.miListenSocket = -1;

    tcp.miErrorFlags = 0;
    tcp.meState = eNew;

    tcp.mSockAddr.sin_addr.s_addr = inet_addr(ipAddress);
    // tcp.mSockAddr.sin_addr.s_addr = htonl(IP_ADDR_ANY);
    tcp.mSockAddr.sin_family = AF_INET;
    tcp.mSockAddr.sin_port = htons(port);

    tcp.mClient.miSocket = -1;
    tcp.mClient.mReceiveBuffer.muSize = 0;
    tcp.mClient.mSendBuffer.muSize = 0;

    return tcp;
}

void initializeTcpServer(TcpServer *pTcpServer) {
    int iProtocol = IPPROTO_IP;
    int iAddressFamily = AF_INET;
    char addr_str[256];

    inet_ntoa_r(((struct sockaddr_in *)&pTcpServer->mSockAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
    ESP_LOGI(TAG, "Initialize for %s:%d", addr_str, ntohs(pTcpServer->mSockAddr.sin_port));

    pTcpServer->miListenSocket = socket(iAddressFamily, SOCK_STREAM, iProtocol);
    if (pTcpServer->miListenSocket < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        pTcpServer->miErrorFlags = pTcpServer->miErrorFlags | TCPSERVERERROR_SOCKET;
        return;
    }
    fcntl(pTcpServer->miListenSocket, F_SETFL, O_NONBLOCK);
    pTcpServer->meState = eSocketCreated;

    ESP_LOGI(TAG, "Socket created");

    // pTcpServer->mSockAddr.sin_addr

    // pTcpServer->mSockAddr.sin_addr.s_addr
    int err = bind(pTcpServer->miListenSocket, (struct sockaddr *)&pTcpServer->mSockAddr, sizeof(pTcpServer->mSockAddr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", iAddressFamily);
        pTcpServer->miErrorFlags = pTcpServer->miErrorFlags | TCPSERVERERROR_BIND;
        return;
    }

    pTcpServer->meState = eSocketBound;
    inet_ntoa_r(((struct sockaddr_in *)&pTcpServer->mSockAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
    ESP_LOGI(TAG, "Socket bound %s:%d", addr_str, ntohs(pTcpServer->mSockAddr.sin_port));
    // ESP_LOGI(TAG, "Socket bound, port %d", pTcpServer->mSockAddr.sin_port);

    err = listen(pTcpServer->miListenSocket, 15);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        pTcpServer->miErrorFlags = pTcpServer->miErrorFlags | TCPSERVERERROR_LISTEN;
        return;
    }
    pTcpServer->meState = eListening;
    ESP_LOGI(TAG, "Listening for connections...");

}
bool acceptConnection(TcpServer *pTcpServer) {
    if (pTcpServer->meState!=eListening) {

        return false;
    }

    // struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
    uint addr_len = sizeof(pTcpServer->mClient.mSockAddr);
    int iSocket = accept(pTcpServer->miListenSocket, (struct sockaddr *)&pTcpServer->mClient.mSockAddr, &addr_len);
    if (iSocket < 0) {
        if (errno!=EAGAIN) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            pTcpServer->miErrorFlags = pTcpServer->miErrorFlags | TCPSERVERERROR_ACCEPT;
            return false;
        } else {
            return false;
        }
    }
    fcntl(pTcpServer->mClient.miSocket, F_SETFL, O_NONBLOCK);
    pTcpServer->meState = eConnected;
    pTcpServer->mClient.miSocket = iSocket;

    // pTcpServer->mClient.mSockAddr = *((struct sockaddr_in*)&source_addr);

    char addr_str[256];
    // Convert ip address to string
    inet_ntoa_r(((struct sockaddr_in *)&pTcpServer->mClient.mSockAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
    ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

    return true;
}
bool receiveData(TcpServer *pTcpServer) {
    if (pTcpServer->meState!=eConnected) return false;
    
    int iReceived;
    uint8_t abReceiveBuffer[128];
        
    iReceived = recv(pTcpServer->mClient.miSocket, abReceiveBuffer, sizeof(abReceiveBuffer) - 1, MSG_DONTWAIT);

    if (iReceived < 0) {
        if ((errno==EAGAIN) || (errno==EWOULDBLOCK)) {
            // ESP_LOGI(TAG, "nothing here");
            return false;
        } else {
            ESP_LOGI(TAG, "Connection closed");
            disconnectAll(pTcpServer);
        }
    } else if (iReceived == 0) {
    } else {
        abReceiveBuffer[iReceived] = 0; // Null-terminate whatever is received and treat it like a string
        ESP_LOGI(TAG, "Received %d bytes: %s", iReceived, abReceiveBuffer);
        // for (int i=0; i<iReceived; i++) {
        //     ESP_LOGI(TAG, "%i", abReceiveBuffer[i]);
        // }

        pushToBuffer(&pTcpServer->mClient.mReceiveBuffer, iReceived, abReceiveBuffer);
        return true;
    }
    return false;
}

bool sendData(TcpServer *pTcpServer) {
    if (pTcpServer->meState!=eConnected) return false;
    uint8_t abSendBuffer[512];
    uint32_t uToWrite = pTcpServer->mClient.mSendBuffer.muSize;
    pullFromBuffer(&pTcpServer->mClient.mSendBuffer, &uToWrite, abSendBuffer);
    if (uToWrite==0) return false;
    uint32_t uOriginalSize = uToWrite;
    while (uToWrite > 0) {
        int written = send(pTcpServer->mClient.miSocket, abSendBuffer + (uOriginalSize - uToWrite), uToWrite, 0);
        if (written < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            disconnectAll(pTcpServer);
            return false;
        }
        uToWrite -= written;
    }
    return true;
}
void disconnectAll(TcpServer *pTcpServer) {
    if (pTcpServer->meState==eConnected && pTcpServer->mClient.miSocket>0) {
        shutdown(pTcpServer->mClient.miSocket, 0);
        close(pTcpServer->mClient.miSocket);
        pTcpServer->mClient.miSocket = -1;
        pTcpServer->meState = eListening;
    }
}
void shutdownServer(TcpServer *pTcpServer) {
    disconnectAll(pTcpServer);
    close(pTcpServer->miListenSocket);
    pTcpServer->miListenSocket = -1;
    pTcpServer->meState = eNew;
}

void printTcpStatus(TcpServer *pTcpServer) {

    char addr_str[256];

    switch (pTcpServer->meState)
    {
    case eNew:
        ESP_LOGI(TAG, "NEW");
        break;
    case eInitialized:
        ESP_LOGI(TAG, "initialized");
        break;
    case eSocketCreated:
        ESP_LOGI(TAG, "socket created");
        break;
    case eSocketBound:
        ESP_LOGI(TAG, "socket bound");
        break;
    case eListening:
        // Convert ip address to string
        inet_ntoa_r(((struct sockaddr_in *)&pTcpServer->mSockAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        ESP_LOGI(TAG, "listening at %s:%i", addr_str, ntohs(pTcpServer->mSockAddr.sin_port));
        break;
    case eConnected:
        // Convert ip address to string
        inet_ntoa_r(((struct sockaddr_in *)&pTcpServer->mClient.mSockAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        ESP_LOGI(TAG, "connected with %s", addr_str);
        break;
    
    default:
        break;
    }
}

void pushToBuffer(DataBuffer *pBuffer, uint32_t byteCount, uint8_t *data) {
    if (pBuffer->muSize>=512) return;

    uint32_t uToPush = byteCount;
    uint32_t uFreeSpace = 512-pBuffer->muSize;
    if (uToPush>uFreeSpace) uToPush = uFreeSpace;
    memcpy(&pBuffer->mabData[ pBuffer->muSize ], data, uToPush);
    pBuffer->muSize += uToPush;
}

void pullFromBuffer(DataBuffer *pBuffer, uint32_t *byteCount, uint8_t *destination) {
    if (pBuffer->muSize==0) {
        *byteCount = 0;
        return;
    }

    memcpy(destination, pBuffer->mabData, pBuffer->muSize);
    destination[pBuffer->muSize] = 0;
    *byteCount = pBuffer->muSize;
    pBuffer->muSize = 0;
}