#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include <Preferences.h>

namespace WiFiManager {
    void initWiFi();
    void startAPMode();
}

#endif // WIFIMANAGER_H