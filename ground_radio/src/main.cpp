// Ground radio bridge. Owned by Communication.
// Vehicle -> laptop: each received telemetry line is written to USB serial as-is
//   (it already starts with <TEAM>C or <TEAM>P, so the app can tell them apart).
// Laptop -> vehicle: each CMD line from USB is sent to the vehicle named in its
//   third field (C or P).
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <string.h>
#include "config.h"

using namespace cavsat;

namespace {

volatile uint32_t received = 0;
char rxLine[MAX_PACKET + 1];
volatile bool rxReady = false;

void onRecv(const uint8_t* mac, const uint8_t* data, int len) {
    // Accept only our two vehicles (X4).
    if (memcmp(mac, CONTAINER_MAC, 6) != 0 && memcmp(mac, POCKETQUBE_MAC, 6) != 0) return;
    if (len <= 0 || (size_t)len > MAX_PACKET || rxReady) return;  // TODO: queue instead of drop
    memcpy(rxLine, data, len);
    rxLine[len] = '\0';
    rxReady = true;
    received++;
}

void addPeer(const uint8_t mac[6]) {
    esp_now_peer_info_t info = {};
    memcpy(info.peer_addr, mac, 6);
    info.channel = ESPNOW_CHANNEL;
    esp_now_add_peer(&info);
}

char cmdBuf[128];
size_t cmdLen = 0;

void sendCommand(const char* line) {
    // CMD,<team>,<C|P>,...
    const char* p = strchr(line, ',');
    if (p) p = strchr(p + 1, ',');
    if (!p) return;
    const uint8_t* target = (p[1] == 'P') ? POCKETQUBE_MAC : CONTAINER_MAC;
    esp_now_send(target, (const uint8_t*)line, strlen(line));
}

} // namespace

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
    esp_now_init();
    esp_now_register_recv_cb(onRecv);
    addPeer(CONTAINER_MAC);
    addPeer(POCKETQUBE_MAC);
    Serial.printf("# ground radio ready, MAC %s, channel %u\n",
                  WiFi.macAddress().c_str(), ESPNOW_CHANNEL);
}

void loop() {
    if (rxReady) {
        // Telemetry lines end in '\r'; add '\n' so the laptop can read by line.
        Serial.print(rxLine);
        Serial.print('\n');
        rxReady = false;
    }
    while (Serial.available()) {
        char ch = (char)Serial.read();
        if (ch == '\n') {
            cmdBuf[cmdLen] = '\0';
            if (cmdLen > 0) sendCommand(cmdBuf);
            cmdLen = 0;
        } else if (ch != '\r' && cmdLen < sizeof(cmdBuf) - 1) {
            cmdBuf[cmdLen++] = ch;
        }
    }
}
