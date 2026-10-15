#pragma once

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

// Current local time as "MM/DD/YYYY HH:MM:SSAM", e.g. "08/06/2024 09:15:22AM".
inline std::string timestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%m/%d/%Y %I:%M:%S%p");
    return out.str();
}
