// Save and load a plain struct in ESP32 flash (NVS) so it survives resets.
#pragma once
#ifdef ARDUINO
#include <Preferences.h>

namespace cavsat {

template <typename T>
class Persist {
public:
    static constexpr uint32_t MAGIC = 0xCA5A7001;

    // Returns true if a saved copy was found (we are recovering from a reset).
    bool begin(const char* ns, T& value) {
        prefs_.begin(ns, false);
        T saved;
        if (prefs_.getBytes("s", &saved, sizeof(T)) == sizeof(T) && saved.magic == MAGIC) {
            value = saved;
            return true;
        }
        value = T{};
        value.magic = MAGIC;
        return false;
    }
    void save(const T& value) { prefs_.putBytes("s", &value, sizeof(T)); }
    void clear() { prefs_.clear(); }

private:
    Preferences prefs_;
};

} // namespace cavsat
#endif
