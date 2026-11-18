#include "tools/clock.hpp"

#include <Arduino.h>
#include <ctime>

uint32_t Clock::s_epochAtSync = 0;
uint32_t Clock::s_syncMillis = 0;
int16_t Clock::s_utcOffsetMinutes = 0;

bool Clock::SetFromBle(uint32_t epochSeconds, int16_t utcOffsetMinutes) {
    if (epochSeconds < MIN_VALID_EPOCH || utcOffsetMinutes < -840 || utcOffsetMinutes > 840) {
        return false;
    }

    s_epochAtSync = epochSeconds;
    s_syncMillis = millis();
    s_utcOffsetMinutes = utcOffsetMinutes;
    return true;
}

bool Clock::IsValid() {
    return s_epochAtSync >= MIN_VALID_EPOCH;
}

uint32_t Clock::GetUtcEpoch() {
    if (!IsValid()) {
        return 0;
    }

    return s_epochAtSync + ((millis() - s_syncMillis) / 1000U);
}

std::string Clock::FormatLocalTime(const char* format) {
    if (!IsValid()) {
        return "";
    }

    const time_t localEpoch = static_cast<time_t>(GetUtcEpoch()) + (s_utcOffsetMinutes * 60);
    struct tm localTime = {};
    gmtime_r(&localEpoch, &localTime);

    char buffer[16] = {};
    strftime(buffer, sizeof(buffer), format, &localTime);
    return std::string(buffer);
}

std::string Clock::GetLocalDateString() {
    return FormatLocalTime("%d/%m/%y");
}

std::string Clock::GetLocalTimeString() {
    return FormatLocalTime("%H:%M");
}
