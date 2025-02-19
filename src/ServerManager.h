#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <ESPAsyncWebServer.h>

namespace ServerManager {
    extern AsyncWebServer server;
    extern AsyncWebSocket ws;
    void initServer();
}

#endif // SERVERMANAGER_H