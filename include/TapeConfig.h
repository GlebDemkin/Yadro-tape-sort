#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>

// Настройки ленты и сортировщика.
// Все задержки задаются в миллисекундах.
struct TapeConfig {
    std::uint64_t readDelayMs = 0;
    std::uint64_t writeDelayMs = 0;
    std::uint64_t moveDelayMs = 0;
    std::uint64_t rewindDelayMs = 0;

    // Лимит памяти для блока, который сортируется в оперативной памяти.
    std::size_t memoryLimitBytes = 1024 * 1024;

    // Лог работы
    bool verbose = false;

    // Загрузка настроек из конфигурационного файла.
    static TapeConfig loadFromFile(const std::filesystem::path& filePath);
};