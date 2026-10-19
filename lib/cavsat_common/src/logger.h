// Append every telemetry line to the SD card so data survives radio dropouts.
#pragma once
#ifdef ARDUINO
#include <SD.h>

namespace cavsat {

class Logger {
public:
    bool begin(uint8_t csPin, const char* path) {
        path_ = path;
        ok_ = SD.begin(csPin);
        return ok_;
    }
    void write(const char* line) {
        if (!ok_) return;
        File f = SD.open(path_, FILE_APPEND);
        if (!f) return;
        f.println(line);
        f.close();  // TODO: keep the file open and flush every N lines if this is too slow
    }
    bool ok() const { return ok_; }
private:
    const char* path_ = "/log.csv";
    bool ok_ = false;
};

} // namespace cavsat
#endif
