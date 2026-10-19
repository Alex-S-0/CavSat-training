// ESP-NOW link: unicast to one peer on the assigned channel, never broadcast (X1-X6).
#pragma once
#ifdef ARDUINO
#include <stddef.h>
#include <stdint.h>

namespace cavsat {

class Radio {
public:
    // peer: the MAC we send to. allowed: MACs we accept packets from.
    bool begin(const uint8_t peer[6], const uint8_t* const allowed[], size_t allowedCount);
    bool send(const char* data, size_t len);
    // Copies the oldest received line into buf. Returns its length, 0 if none.
    size_t receive(char* buf, size_t len, uint8_t fromMac[6] = nullptr);
    uint32_t sendFailures() const;
};

} // namespace cavsat
#endif
