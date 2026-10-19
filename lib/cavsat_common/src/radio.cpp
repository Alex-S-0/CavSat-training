#ifdef ARDUINO
#include "radio.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <string.h>

namespace cavsat {

namespace {

constexpr size_t QUEUE_LEN = 8;
struct Slot { uint8_t mac[6]; char data[MAX_PACKET + 1]; size_t len; };

Slot queue_[QUEUE_LEN];
volatile size_t head_ = 0, tail_ = 0;
volatile uint32_t sendFailures_ = 0;
const uint8_t* const* allowed_ = nullptr;
size_t allowedCount_ = 0;
uint8_t peer_[6];
portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;

bool isAllowed(const uint8_t* mac) {
    for (size_t i = 0; i < allowedCount_; ++i)
        if (memcmp(mac, allowed_[i], 6) == 0) return true;
    return false;
}

// Runs in the Wi-Fi task: copy and return quickly.
void onRecv(const uint8_t* mac, const uint8_t* data, int len) {
    if (!isAllowed(mac) || len <= 0 || (size_t)len > MAX_PACKET) return;
    portENTER_CRITICAL(&mux_);
    size_t next = (head_ + 1) % QUEUE_LEN;
    if (next != tail_) {
        Slot& s = queue_[head_];
        memcpy(s.mac, mac, 6);
        memcpy(s.data, data, len);
        s.data[len] = '\0';
        s.len = len;
        head_ = next;
    }
    portEXIT_CRITICAL(&mux_);
}

void onSent(const uint8_t*, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) sendFailures_++;
}

} // namespace

bool Radio::begin(const uint8_t peer[6], const uint8_t* const allowed[], size_t allowedCount) {
    memcpy(peer_, peer, 6);
    allowed_ = allowed;
    allowedCount_ = allowedCount;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
    if (esp_now_init() != ESP_OK) return false;
    esp_now_register_recv_cb(onRecv);
    esp_now_register_send_cb(onSent);

    esp_now_peer_info_t info = {};
    memcpy(info.peer_addr, peer_, 6);
    info.channel = ESPNOW_CHANNEL;
    info.encrypt = false;
    return esp_now_add_peer(&info) == ESP_OK;
}

bool Radio::send(const char* data, size_t len) {
    if (len > MAX_PACKET) return false;
    return esp_now_send(peer_, (const uint8_t*)data, len) == ESP_OK;
}

size_t Radio::receive(char* buf, size_t len, uint8_t fromMac[6]) {
    size_t n = 0;
    portENTER_CRITICAL(&mux_);
    if (tail_ != head_) {
        Slot& s = queue_[tail_];
        n = s.len < len - 1 ? s.len : len - 1;
        memcpy(buf, s.data, n);
        buf[n] = '\0';
        if (fromMac) memcpy(fromMac, s.mac, 6);
        tail_ = (tail_ + 1) % QUEUE_LEN;
    }
    portEXIT_CRITICAL(&mux_);
    return n;
}

uint32_t Radio::sendFailures() const { return sendFailures_; }

} // namespace cavsat
#endif
