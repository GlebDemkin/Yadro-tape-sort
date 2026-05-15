#include "TapeSorter.h"

#include "FileTape.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

TapeSorter::TapeSorter(
    TapeConfig config,
    std::filesystem::path tmpDirectory
) : config_(config),
    tmpDirectory_(std::move(tmpDirectory)) {
        // Создание папки для временных лент.
        // Если папка уже существует, ошибки не будет.
        std::filesystem::create_directories(tmpDirectory_);

        if (config_.verbose) {
            std::cout << "Created tmp directory: " << tmpDirectory_ << '\n';
        }
    }

void TapeSorter::sort(ITape& inputTape, ITape& outputTape) {
    // Входная и выходная ленты должны иметь одинаковую длину.
    // Иначе мы не сможем записать все отсортированные элементы.
    if (inputTape.size() != outputTape.size()) {
        throw std::runtime_error("Input and output tapes must have the same size");
    }

    // Если входная лента пустая, сортировать нечего.
    if (inputTape.size() == 0) {
        return;
    }

    // level — номер уровня временных лент.
    // level_0 — первые отсортированные блоки, полученные из входной ленты.
    std::size_t level = 0;

    // Создаём начальные отсортированные блоки.
    // Функция возвращает количество созданных временных лент.
    std::size_t runCount = createSortedRuns(inputTape);

    // Попарное слияние временных лент.
    // После каждого уровня количество временных лент уменьшается.
    while (runCount > 1) {
        runCount = mergeLevel(level, runCount);
        ++level;
    }

    // Когда осталась одна временная лента, она уже полностью отсортирована.
    const std::filesystem::path finalRunPath = makeRunPath(level, 0);

    // Копируем итоговую временную ленту в выходную ленту.
    copyRunToOutput(finalRunPath, outputTape);

    // Удаляем последнюю временную ленту после копирования результата.
    removeFileIfExists(finalRunPath);
}

std::size_t TapeSorter::createSortedRuns(ITape& inputTape) {
    // blockSize — сколько элементов можно держать в оперативной памяти.
    const std::size_t blockSize = calculateBlockSize();

    // Количество созданных временных лент на уровне 0.
    std::size_t runCount = 0;

    // Перед чтением входной ленты перематываемся в начало.
    inputTape.rewindToBegin();

    while (!inputTape.isEnd()) {
        // block — небольшой кусок входной ленты.
        // Он помещается в оперативную память.
        std::vector<ITape::ValueType> block;
        block.reserve(blockSize);

        // Чтение блока, который помещается в оперативную память.
        for (
            std::size_t readCount = 0;
            readCount < blockSize && !inputTape.isEnd();
            ++readCount
            ) {
            block.push_back(inputTape.read());
            inputTape.moveRight();
        }

        // Сортируем только маленький блок, а не всю входную ленту.
        std::sort(block.begin(), block.end());

        // Формируем имя временной ленты уровня 0.
        // Например: tmp/level_0_run_0.bin.
        const std::filesystem::path runPath = makeRunPath(0, runCount);

        // Вывод информации о создании временной ленты
        if (config_.verbose) {
            std::cout << "Created run: " << runPath << '\n';
        }

        // Создаём временную ленту для отсортированного блока.
        FileTape runTape(
            runPath,
            config_,
            FileTape::OpenMode::CreateNew,
            block.size()
        );

        // Перед записью блока ставим головку временной ленты в начало.
        runTape.rewindToBegin();

        // Записывание отсортированного блока во временную ленту.
        for (std::size_t index = 0; index < block.size(); ++index) {
            runTape.write(block[index]);

            // После записи двигаемся вправо, если это не последняя ячейка.
            if (index + 1 < block.size()) {
                runTape.moveRight();
            }
        }

        // Увеличиваем количество созданных временных лент.
        ++runCount;
    }

    return runCount;
}

std::size_t TapeSorter::mergeLevel(
    std::size_t level,
    std::size_t runCount
) {
    // Количество временных лент, которое получится на следующем уровне.
    std::size_t nextRunCount = 0;

    // Обрабатываем временные ленты парами:
    // run_0 + run_1, run_2 + run_3 и так далее.
    for (std::size_t runIndex = 0; runIndex < runCount; runIndex += 2) {
        // Левая временная лента текущей пары.
        const std::filesystem::path leftRunPath =
            makeRunPath(level, runIndex);

        // Путь для результата слияния на следующем уровне.
        const std::filesystem::path nextRunPath =
            makeRunPath(level + 1, nextRunCount);

        // Если лента осталась без пары, переносим её на следующий уровень.
        // Например, если было 5 лент, пятая просто переименуется.
        if (runIndex + 1 >= runCount) {
            removeFileIfExists(nextRunPath);
            std::filesystem::rename(leftRunPath, nextRunPath);

            ++nextRunCount;
            continue;
        }

        // Правая временная лента текущей пары.
        const std::filesystem::path rightRunPath =
            makeRunPath(level, runIndex + 1);

        // Вывод информации о слиянии временных лент.
        if (config_.verbose) {
            std::cout
                << "Merged: "
                << leftRunPath
                << " + "
                << rightRunPath
                << " -> "
                << nextRunPath
                << '\n';
        }

        // Сливаем две отсортированные временные ленты в одну.
        mergeTwoRuns(leftRunPath, rightRunPath, nextRunPath);

        // После слияния старые временные ленты больше не нужны.
        removeFileIfExists(leftRunPath);
        removeFileIfExists(rightRunPath);

        ++nextRunCount;
    }

    return nextRunCount;
}

void TapeSorter::mergeTwoRuns(
    const std::filesystem::path& leftRunPath,
    const std::filesystem::path& rightRunPath,
    const std::filesystem::path& mergedRunPath
) {
    // Открываем две отсортированные временные ленты.
    FileTape leftRunTape(leftRunPath, config_);
    FileTape rightRunTape(rightRunPath, config_);

    // Размер результата равен сумме размеров двух временных лент.
    const std::size_t mergedSize =
        leftRunTape.size() + rightRunTape.size();

    // Создаём временную ленту для результата слияния.
    FileTape mergedRunTape(
        mergedRunPath,
        config_,
        FileTape::OpenMode::CreateNew,
        mergedSize
    );

    // Все три ленты ставим в начало.
    leftRunTape.rewindToBegin();
    rightRunTape.rewindToBegin();
    mergedRunTape.rewindToBegin();

    // Эти флаги показывают, есть ли ещё текущий элемент в каждой ленте.
    bool hasLeftRunValue = !leftRunTape.isEnd();
    bool hasRightRunValue = !rightRunTape.isEnd();

    // В памяти храним только два текущих значения.
    ITape::ValueType leftRunValue = 0;
    ITape::ValueType rightRunValue = 0;

    // Читаем первое значение из левой ленты, если она не пустая.
    if (hasLeftRunValue) {
        leftRunValue = leftRunTape.read();
    }

    // Читаем первое значение из правой ленты, если она не пустая.
    if (hasRightRunValue) {
        rightRunValue = rightRunTape.read();
    }

    // Слияние двух отсортированных временных лент.
    // На каждом шаге записываем меньшее из двух текущих значений.
    while (hasLeftRunValue && hasRightRunValue) {
        if (leftRunValue <= rightRunValue) {
            // Если левое значение меньше или равно, записываем его.
            writeAndMoveIfNeeded(mergedRunTape, leftRunValue);

            // После записи двигаем левую ленту к следующей ячейке.
            leftRunTape.moveRight();
            hasLeftRunValue = !leftRunTape.isEnd();

            // Если левая лента не закончилась, читаем новое текущее значение.
            if (hasLeftRunValue) {
                leftRunValue = leftRunTape.read();
            }
        }
        else {
            // Если правое значение меньше, записываем его.
            writeAndMoveIfNeeded(mergedRunTape, rightRunValue);

            // После записи двигаем правую ленту к следующей ячейке.
            rightRunTape.moveRight();
            hasRightRunValue = !rightRunTape.isEnd();

            // Если правая лента не закончилась, читаем новое текущее значение.
            if (hasRightRunValue) {
                rightRunValue = rightRunTape.read();
            }
        }
    }

    // Дописывание остатка левой ленты.
    // Сюда попадаем, если правая лента уже закончилась.
    while (hasLeftRunValue) {
        writeAndMoveIfNeeded(mergedRunTape, leftRunValue);

        leftRunTape.moveRight();
        hasLeftRunValue = !leftRunTape.isEnd();

        if (hasLeftRunValue) {
            leftRunValue = leftRunTape.read();
        }
    }

    // Дописывание остатка правой ленты.
    // Сюда попадаем, если левая лента уже закончилась.
    while (hasRightRunValue) {
        writeAndMoveIfNeeded(mergedRunTape, rightRunValue);

        rightRunTape.moveRight();
        hasRightRunValue = !rightRunTape.isEnd();

        if (hasRightRunValue) {
            rightRunValue = rightRunTape.read();
        }
    }
}

void TapeSorter::copyRunToOutput(
    const std::filesystem::path& runPath,
    ITape& outputTape
) {
    // Открываем итоговую отсортированную временную ленту.
    FileTape sortedRunTape(runPath, config_);

    // Размер итоговой временной ленты должен совпадать с размером output.
    if (sortedRunTape.size() != outputTape.size()) {
        throw std::runtime_error("Sorted run size does not match output tape size");
    }

    // Перед копированием ставим обе ленты в начало.
    sortedRunTape.rewindToBegin();
    outputTape.rewindToBegin();

    // Копирование итоговой ленты в выходной файл.
    while (!sortedRunTape.isEnd()) {
        const ITape::ValueType value = sortedRunTape.read();

        writeAndMoveIfNeeded(outputTape, value);

        sortedRunTape.moveRight();
    }
}

std::filesystem::path TapeSorter::makeRunPath(
    std::size_t level,
    std::size_t runIndex
) const {
    // Имя временной ленты строится по уровню и номеру.
    // Например: level_0_run_2.bin.
    const std::string fileName =
        "level_"
        + std::to_string(level)
        + "_run_"
        + std::to_string(runIndex)
        + ".bin";

    // Возвращаем полный путь внутри папки tmp.
    return tmpDirectory_ / fileName;
}

std::size_t TapeSorter::calculateBlockSize() const {
    // Один элемент ленты занимает sizeof(ITape::ValueType) байт.
    // Делим лимит памяти на размер одного элемента.
    const std::size_t blockSize =
        config_.memoryLimitBytes / sizeof(ITape::ValueType);

    // Если памяти не хватает даже на один элемент, сортировка невозможна.
    if (blockSize == 0) {
        throw std::runtime_error(
            "Memory limit is too small to store one tape value"
        );
    }

    return blockSize;
}

void TapeSorter::writeAndMoveIfNeeded(
    ITape& tape,
    ITape::ValueType value
) {
    // Записываем значение в текущую ячейку.
    tape.write(value);

    // Двигаем ленту вправо, если текущая ячейка не последняя.
    // Так мы избегаем выхода за границу ленты после последней записи.
    if (tape.position() + 1 < tape.size()) {
        tape.moveRight();
    }
}

void TapeSorter::removeFileIfExists(const std::filesystem::path& filePath) const {
    if (std::filesystem::exists(filePath)) {
        std::filesystem::remove(filePath);

        // Вывод информации об удалении.
        if (config_.verbose) {
            std::cout << "Removed: " << filePath << '\n';
        }
    }
}