#include "TapeConfig.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

    // Удаление пробелов по краям строки.
    std::string trim(const std::string& text) {
        const std::string spaces = " \t\n\r";

        const std::size_t firstSymbol = text.find_first_not_of(spaces);

        // Проверка на пустую строку
        if (firstSymbol == std::string::npos) {
            return "";
        }

        const std::size_t lastSymbol = text.find_last_not_of(spaces);

        return text.substr(firstSymbol, lastSymbol - firstSymbol + 1);
    }

    // Преобразование строки в беззнаковое число.
    std::uint64_t parseUnsignedNumber(const std::string& text) {
        try {
            return std::stoull(text);
        }
        catch (const std::exception&) {
            throw std::runtime_error("Invalid number in config: " + text);
        }
    }

    bool parseBool(const std::string& text) {
        if (text == "true" || text == "1") {
            return true;
        }

        if (text == "false" || text == "0") {
            return false;
        }

        throw std::runtime_error("Invalid bool value in config: " + text);
    }

} // С помощью namespace изолируем эти функции от всего проекта, кроме этого файла

TapeConfig TapeConfig::loadFromFile(const std::filesystem::path& filePath) {
    std::ifstream input(filePath);

    if (!input.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filePath.string());
    }

    TapeConfig config;

    std::string line;

    while (std::getline(input, line)) {
        line = trim(line);

        // Пропуск пустых строк и комментариев.
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const std::size_t separatorPosition = line.find('=');

        if (separatorPosition == std::string::npos) {
            throw std::runtime_error("Invalid config line: " + line);
        }

        // Разделение на ключ и значение
        const std::string key = trim(line.substr(0, separatorPosition));
        const std::string value = trim(line.substr(separatorPosition + 1));

        if (key == "read_delay_ms") {
            config.readDelayMs = parseUnsignedNumber(value);
        }
        else if (key == "write_delay_ms") {
            config.writeDelayMs = parseUnsignedNumber(value);
        }
        else if (key == "move_delay_ms") {
            config.moveDelayMs = parseUnsignedNumber(value);
        }
        else if (key == "rewind_delay_ms") {
            config.rewindDelayMs = parseUnsignedNumber(value);
        }
        else if (key == "memory_limit_bytes") {
            config.memoryLimitBytes = static_cast<std::size_t>(parseUnsignedNumber(value));
        }
        else if (key == "verbose") {
            config.verbose = parseBool(value);
        }
        else {
            throw std::runtime_error("Unknown config key: " + key);
        }
    }

    return config;
}