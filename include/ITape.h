#pragma once

#include <cstddef>
#include <cstdint>

// Общий интерфейс ленты.
// Сортировщик работает только с этим интерфейсом, а не напрямую с файлами.
class ITape {
public:
    using ValueType = std::int32_t;

    virtual ~ITape() = default;

    // Чтение значений из текущей ячейки.
    virtual ValueType read() = 0;

    // Записывание значений в текущую ячейку.
    virtual void write(ValueType value) = 0;

    // Сдвиг ленты на одну позицию влево.
    virtual void moveLeft() = 0;

    // Сдвиг ленты на одну позицию вправо.
    virtual void moveRight() = 0;

    // Перемотка в начало ленты.
    virtual void rewindToBegin() = 0;

    // Перемотка на последнюю ячейку ленты.
    virtual void rewindToEnd() = 0;

    // Проверка: головка стоит в начале.
    virtual bool isAtBegin() const = 0;

    // Проверка: выход за последнюю ячейку.
    virtual bool isEnd() const = 0;

    // Текущая позиция головки.
    virtual std::size_t position() const = 0;

    // Количество ячеек на ленте.
    virtual std::size_t size() const = 0;
};