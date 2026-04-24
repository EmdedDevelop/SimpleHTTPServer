#include <string>
#include <map>
#include <chrono>
#include "html_funcs.h"


std::string replacePlaceholders(const std::string& templateStr,
    const std::map<std::string, std::string>& replacements) {
    std::string result = templateStr;

    for (const auto& [placeholder, value] : replacements) {
        size_t pos = result.find(placeholder);
        while (pos != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
            pos = result.find(placeholder, pos + value.length());
        }
    }

    return result;
}


// Функция для получения текущей даты и времени
std::string GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    struct tm timeInfo;
    localtime_s(&timeInfo, &time);

    // DD-MM-YYYY HH:MM:SS
    ss << std::put_time(&timeInfo, "%d %B %Y %H:%M:%S");
    return ss.str();
}