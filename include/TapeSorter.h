#pragma once

#include "ITape.h"
#include "TapeConfig.h"

#include <filesystem>

// Сортировщик ленты.
// Используется внешняя сортировка слиянием.
class TapeSorter {
public:
    explicit TapeSorter(
        TapeConfig config,
        std::filesystem::path tmpDirectory = "tmp"
    );

    // Сортировка входной ленты в выходную ленту.
    void sort(ITape& inputTape, ITape& outputTape);

private:
    // Создание начальных отсортированных временных лент.
    std::size_t createSortedRuns(ITape& inputTape);

    // Слияние всех временных лент одного уровня.
    std::size_t mergeLevel(
        std::size_t level,
        std::size_t runCount
    );

    // Слияние двух временных лент.
    void mergeTwoRuns(
        const std::filesystem::path& leftRunPath,
        const std::filesystem::path& rightRunPath,
        const std::filesystem::path& mergedRunPath
    );

    // Копирование итоговой временной ленты в выходную ленту.
    void copyRunToOutput(
        const std::filesystem::path& runPath,
        ITape& outputTape
    );

    // Формирование имени временной ленты.
    std::filesystem::path makeRunPath(
        std::size_t level,
        std::size_t runIndex
    ) const;

    // Расчёт количества элементов в одном блоке.
    std::size_t calculateBlockSize() const;

    // Записывание значения и сдвиг ленты при необходимости.
    static void writeAndMoveIfNeeded(
        ITape& tape,
        ITape::ValueType value
    );

    // Удаление файла, если он существует.
    void removeFileIfExists(const std::filesystem::path& filePath) const;

private:
    TapeConfig config_;
    std::filesystem::path tmpDirectory_;
};