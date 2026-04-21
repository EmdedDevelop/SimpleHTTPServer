#include <string>
#include <map>
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