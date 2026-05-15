#include "FileTape.h"
#include "TapeConfig.h"
#include "TapeSorter.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

    // Записывание тестовой ленты в бинарный файл.
    void writeBinaryTapeFile(
        const std::filesystem::path& filePath,
        const std::vector<std::int32_t>& values
    ) {
        const std::filesystem::path parentPath = filePath.parent_path();

        if (!parentPath.empty()) {
            std::filesystem::create_directories(parentPath);
        }

        std::ofstream output(filePath, std::ios::binary | std::ios::trunc);

        for (const std::int32_t value : values) {
            output.write(
                reinterpret_cast<const char*>(&value),
                sizeof(std::int32_t)
            );
        }
    }

    // Чтение тестовой ленты из бинарного файла.
    std::vector<std::int32_t> readBinaryTapeFile(
        const std::filesystem::path& filePath
    ) {
        std::ifstream input(filePath, std::ios::binary);

        std::vector<std::int32_t> values;

        std::int32_t value = 0;

        while (
            input.read(
                reinterpret_cast<char*>(&value),
                sizeof(std::int32_t)
            )
            ) {
            values.push_back(value);
        }

        return values;
    }

    // Создание настроек для тестов.
    TapeConfig makeTestConfig(std::size_t memoryLimitBytes) {
        TapeConfig config;

        config.readDelayMs = 0;
        config.writeDelayMs = 0;
        config.moveDelayMs = 0;
        config.rewindDelayMs = 0;
        config.memoryLimitBytes = memoryLimitBytes;

        return config;
    }

} // namespace

TEST(TapeSorterTests, SortsValuesWithDuplicates) {
    const std::filesystem::path testDirectory = "tmp/tests/duplicates";
    std::filesystem::remove_all(testDirectory);
    std::filesystem::create_directories(testDirectory);

    const std::filesystem::path inputPath = testDirectory / "input.bin";
    const std::filesystem::path outputPath = testDirectory / "output.bin";
    const std::filesystem::path workDirectory = testDirectory / "work";

    writeBinaryTapeFile(inputPath, { 2, 4, 3, 1, 3, 5 });

    TapeConfig config = makeTestConfig(3 * sizeof(std::int32_t));

    FileTape inputTape(inputPath, config);

    FileTape outputTape(
        outputPath,
        config,
        FileTape::OpenMode::CreateNew,
        inputTape.size()
    );

    TapeSorter sorter(config, workDirectory);
    sorter.sort(inputTape, outputTape);

    const std::vector<std::int32_t> actual =
        readBinaryTapeFile(outputPath);

    const std::vector<std::int32_t> expected =
    { 1, 2, 3, 3, 4, 5 };

    EXPECT_EQ(actual, expected);
}

TEST(TapeSorterTests, SortsNegativeValues) {
    const std::filesystem::path testDirectory = "tmp/tests/negative";
    std::filesystem::remove_all(testDirectory);
    std::filesystem::create_directories(testDirectory);

    const std::filesystem::path inputPath = testDirectory / "input.bin";
    const std::filesystem::path outputPath = testDirectory / "output.bin";
    const std::filesystem::path workDirectory = testDirectory / "work";

    writeBinaryTapeFile(inputPath, { 10, -5, 3, 0, -5, 8 });

    TapeConfig config = makeTestConfig(2 * sizeof(std::int32_t));

    FileTape inputTape(inputPath, config);

    FileTape outputTape(
        outputPath,
        config,
        FileTape::OpenMode::CreateNew,
        inputTape.size()
    );

    TapeSorter sorter(config, workDirectory);
    sorter.sort(inputTape, outputTape);

    const std::vector<std::int32_t> actual =
        readBinaryTapeFile(outputPath);

    const std::vector<std::int32_t> expected =
    { -5, -5, 0, 3, 8, 10 };

    EXPECT_EQ(actual, expected);
}

TEST(TapeSorterTests, SortsAlreadySortedValues) {
    const std::filesystem::path testDirectory = "tmp/tests/already_sorted";
    std::filesystem::remove_all(testDirectory);
    std::filesystem::create_directories(testDirectory);

    const std::filesystem::path inputPath = testDirectory / "input.bin";
    const std::filesystem::path outputPath = testDirectory / "output.bin";
    const std::filesystem::path workDirectory = testDirectory / "work";

    writeBinaryTapeFile(inputPath, { 1, 2, 3, 4, 5 });

    TapeConfig config = makeTestConfig(2 * sizeof(std::int32_t));

    FileTape inputTape(inputPath, config);

    FileTape outputTape(
        outputPath,
        config,
        FileTape::OpenMode::CreateNew,
        inputTape.size()
    );

    TapeSorter sorter(config, workDirectory);
    sorter.sort(inputTape, outputTape);

    const std::vector<std::int32_t> actual =
        readBinaryTapeFile(outputPath);

    const std::vector<std::int32_t> expected =
    { 1, 2, 3, 4, 5 };

    EXPECT_EQ(actual, expected);
}

TEST(TapeSorterTests, SortsReverseSortedValues) {
    const std::filesystem::path testDirectory = "tmp/tests/reverse_sorted";
    std::filesystem::remove_all(testDirectory);
    std::filesystem::create_directories(testDirectory);

    const std::filesystem::path inputPath = testDirectory / "input.bin";
    const std::filesystem::path outputPath = testDirectory / "output.bin";
    const std::filesystem::path workDirectory = testDirectory / "work";

    writeBinaryTapeFile(inputPath, { 5, 4, 3, 2, 1 });

    TapeConfig config = makeTestConfig(2 * sizeof(std::int32_t));

    FileTape inputTape(inputPath, config);

    FileTape outputTape(
        outputPath,
        config,
        FileTape::OpenMode::CreateNew,
        inputTape.size()
    );

    TapeSorter sorter(config, workDirectory);
    sorter.sort(inputTape, outputTape);

    const std::vector<std::int32_t> actual =
        readBinaryTapeFile(outputPath);

    const std::vector<std::int32_t> expected =
    { 1, 2, 3, 4, 5 };

    EXPECT_EQ(actual, expected);
}

TEST(TapeSorterTests, ThrowsWhenMemoryLimitIsTooSmall) {
    const std::filesystem::path testDirectory = "tmp/tests/small_memory";
    std::filesystem::remove_all(testDirectory);
    std::filesystem::create_directories(testDirectory);

    const std::filesystem::path inputPath = testDirectory / "input.bin";
    const std::filesystem::path outputPath = testDirectory / "output.bin";
    const std::filesystem::path workDirectory = testDirectory / "work";

    writeBinaryTapeFile(inputPath, { 3, 2, 1 });

    TapeConfig config = makeTestConfig(1);

    FileTape inputTape(inputPath, config);

    FileTape outputTape(
        outputPath,
        config,
        FileTape::OpenMode::CreateNew,
        inputTape.size()
    );

    TapeSorter sorter(config, workDirectory);

    EXPECT_THROW(
        sorter.sort(inputTape, outputTape),
        std::runtime_error
    );
}