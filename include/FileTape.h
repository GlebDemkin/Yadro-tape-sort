#pragma once

#include "ITape.h"
#include "TapeConfig.h"

#include <filesystem>
#include <fstream>

// Файловая реализация ленты.
// Внутри используется бинарный файл, но снаружи остаётся интерфейс ленты.
class FileTape final : public ITape {
public:
    enum class OpenMode {
        ReadWriteExisting,
        CreateNew
    };

    FileTape(
        const std::filesystem::path& filePath,
        const TapeConfig& config,
        OpenMode openMode = OpenMode::ReadWriteExisting,
        std::size_t cellCount = 0
    );

    ~FileTape() override;

    FileTape(const FileTape&) = delete;
    FileTape& operator=(const FileTape&) = delete;

    // Чтение значения из текущей ячейки.
    ValueType read() override;

    // Записывание значения в текущую ячейку.
    void write(ValueType value) override;

    // Сдвиг ленты на одну позицию влево.
    void moveLeft() override;

    // Сдвиг ленты на одну позицию вправо.
    void moveRight() override;

    // Перемотка в начало ленты.
    void rewindToBegin() override;

    // Перемотка на последнюю ячейку ленты.
    void rewindToEnd() override;

    // Проверка: головка стоит в начале.
    bool isAtBegin() const override;

    // Проверка: выход за последнюю ячейку.
    bool isEnd() const override;

    // Текущая позиция головки.
    std::size_t position() const override;

    // Количество ячеек на ленте.
    std::size_t size() const override;

private:
    // Подготовка существующего файла ленты.
    void prepareExistingFile();

    // Создание нового файла ленты.
    void createEmptyFile(std::size_t cellCount);

    // Ожидание задержки операции.
    void wait(std::uint64_t delayMs) const;

    // Смещение текущей ячейки в байтах.
    std::streamoff currentByteOffset() const;

private:
    std::filesystem::path filePath_;
    TapeConfig config_;

    std::fstream file_;

    std::size_t currentPosition_ = 0;
    std::size_t tapeSize_ = 0;
};