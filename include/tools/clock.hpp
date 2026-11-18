#pragma once

#include <stdint.h>
#include <string>

// Runtime clock synchronized by a paired mobile controller. The ESP32 has no
// battery-backed RTC, so this intentionally becomes invalid after a reboot
// until a controller supplies a fresh timestamp.
class Clock {
  public:
    static constexpr uint32_t MIN_VALID_EPOCH = 1577836800; // 2020-01-01 UTC

    static bool SetFromBle(uint32_t epochSeconds, int16_t utcOffsetMinutes = 0);
    static bool IsValid();
    static uint32_t GetUtcEpoch();
    static std::string GetLocalDateString();
    static std::string GetLocalTimeString();

  private:
    static uint32_t s_epochAtSync;
    static uint32_t s_syncMillis;
    static int16_t s_utcOffsetMinutes;
    static std::string FormatLocalTime(const char* format);
};
