#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

    // Вывод подсказки по использованию программы.
    void printUsage(const char* programName) {
        std::cerr
            << "Usage:\n"
            << "  " << programName << " encode <input_txt> <output_bin>\n"
            << "  " << programName << " decode <input_bin> <output_txt>\n"
            << "  " << programName << " print <input_bin>\n";
    }

    // Создание папки для выходного файла.
    void createDirectoryForFile(const std::string& filePath) {
        const std::filesystem::path path(filePath);
        const std::filesystem::path parentPath = path.parent_path();

        if (!parentPath.empty()) {
            std::filesystem::create_directories(parentPath);
        }
    }

    // Вывод чисел в консоль.
    void printValues(
        const std::string& title,
        const std::vector<std::int32_t>& values
    ) {
        std::cout << title;

        for (std::size_t index = 0; index < values.size(); ++index) {
            if (index > 0) {
                std::cout << ' ';
            }

            std::cout << values[index];
        }

        std::cout << '\n';
    }

    // Преобразование обычного текстового файла в бинарную ленту.
    void encodeTextToBinary(
        const std::string& inputTextPath,
        const std::string& outputBinaryPath
    ) {
        // Открытие текстового файла с исходными числами.
        std::ifstream input(inputTextPath);

        if (!input.is_open()) {
            throw std::runtime_error("Cannot open input text file: " + inputTextPath);
        }

        // Создание папки для бинарного файла, если она указана в пути.
        createDirectoryForFile(outputBinaryPath);

        // Создание бинарного файла ленты.
        // Если файл уже существует, он будет очищен.
        std::ofstream output(
            outputBinaryPath,
            std::ios::binary | std::ios::trunc
        );

        if (!output.is_open()) {
            throw std::runtime_error("Cannot create output binary file: " + outputBinaryPath);
        }

        // Вектор нужен только для вывода прочитанных чисел в консоль.
        std::vector<std::int32_t> values;

        std::int32_t value = 0;

        // Чтение чисел из input.txt.
        while (input >> value) {
            values.push_back(value);

            // Запись числа в бинарный файл как std::int32_t.
            output.write(
                reinterpret_cast<const char*>(&value),
                sizeof(value)
            );

            if (output.fail()) {
                throw std::runtime_error("Cannot write value to binary file");
            }
        }

        // Вывод исходных чисел в консоль.
        printValues("Input values: ", values);

        std::cout << "Created binary tape: " << outputBinaryPath << '\n';
    }

    // Преобразование бинарной ленты в обычный текстовый файл.
    void decodeBinaryToText(
        const std::string& inputBinaryPath,
        const std::string& outputTextPath
    ) {
        // Открытие бинарной ленты.
        std::ifstream input(inputBinaryPath, std::ios::binary);

        if (!input.is_open()) {
            throw std::runtime_error("Cannot open input binary file: " + inputBinaryPath);
        }

        // Создание папки для текстового файла, если она указана в пути.
        createDirectoryForFile(outputTextPath);

        // Создание текстового файла для результата.
        std::ofstream output(outputTextPath, std::ios::trunc);

        if (!output.is_open()) {
            throw std::runtime_error("Cannot create output text file: " + outputTextPath);
        }

        // Здесь храним числа, которые прочитали из бинарной ленты.
        std::vector<std::int32_t> values;

        std::int32_t value = 0;

        // Чтение чисел из бинарного файла.
        while (
            input.read(
                reinterpret_cast<char*>(&value),
                sizeof(value)
            )
            ) {
            values.push_back(value);
        }

        // Запись чисел в обычный текстовый файл.
        for (std::size_t index = 0; index < values.size(); ++index) {
            if (index > 0) {
                output << ' ';
            }

            output << values[index];
        }

        output << '\n';

        // Вывод результата в консоль.
        printValues("Output values: ", values);

        std::cout << "Created text file: " << outputTextPath << '\n';
    }

    // Вывод бинарной ленты в консоль без создания текстового файла.
    void printBinaryTape(const std::string& inputBinaryPath) {
        // Открытие бинарной ленты.
        std::ifstream input(inputBinaryPath, std::ios::binary);

        if (!input.is_open()) {
            throw std::runtime_error("Cannot open input binary file: " + inputBinaryPath);
        }

        // Здесь храним числа, которые прочитали из бинарной ленты.
        std::vector<std::int32_t> values;

        std::int32_t value = 0;

        // Чтение чисел из бинарного файла.
        while (
            input.read(
                reinterpret_cast<char*>(&value),
                sizeof(value)
            )
            ) {
            values.push_back(value);
        }

        // Вывод чисел в консоль.
        printValues("Tape values: ", values);
    }

} // namespace

int main(int argc, char* argv[]) {
    try {
        // Проверка, что пользователь передал хотя бы команду.
        if (argc < 2) {
            printUsage(argv[0]);
            return 1;
        }

        const std::string command = argv[1];

        // Команда encode: txt -> bin.
        if (command == "encode") {
            if (argc != 4) {
                printUsage(argv[0]);
                return 1;
            }

            encodeTextToBinary(argv[2], argv[3]);
            return 0;
        }

        // Команда decode: bin -> txt.
        if (command == "decode") {
            if (argc != 4) {
                printUsage(argv[0]);
                return 1;
            }

            decodeBinaryToText(argv[2], argv[3]);
            return 0;
        }

        // Команда print: вывод bin в консоль.
        if (command == "print") {
            if (argc != 3) {
                printUsage(argv[0]);
                return 1;
            }

            printBinaryTape(argv[2]);
            return 0;
        }

        // Если команда неизвестна, выводим подсказку.
        printUsage(argv[0]);
        return 1;
    }
    catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }
}