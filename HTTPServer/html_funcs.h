#pragma once


std::string replacePlaceholders(const std::string& templateStr,
    const std::map<std::string, std::string>& replacements);


std::string GetCurrentTimestamp();
