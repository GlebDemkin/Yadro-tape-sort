#include "FileTape.h"

#include <chrono>
#include <stdexcept>
#include <thread>

FileTape::FileTape(
    const std::filesystem::path& filePath,
    const TapeConfig& config,
    OpenMode openMode,
    std::size_t cellCount
) : filePath_(filePath), 
    config_(config) {
        if (openMode == OpenMode::CreateNew) {
            createEmptyFile(cellCount);
        }
        else {
            prepareExistingFile();
        }

        // Открываем файл как бинарный сразу на чтение и запись
        file_.open(filePath_, std::ios::binary | std::ios::in | std::ios::out);

        if (!file_.is_open()) {
            throw std::runtime_error("Cannot open tape file: " + filePath_.string());
        }

        currentPosition_ = 0;
    }

FileTape::~FileTape() {
    if (file_.is_open()) {
        file_.close();
    }
}

void FileTape::prepareExistingFile() {
    if (!std::filesystem::exists(filePath_)) {
        throw std::runtime_error("Tape file does not exist: " + filePath_.string());
    }

    const std::uintmax_t fileSizeInBytes = std::filesystem::file_size(filePath_);

    if (fileSizeInBytes % sizeof(ValueType) != 0) {
        throw std::runtime_error(
            "Invalid tape file size. File size must be divisible by 4 bytes: "
            + filePath_.string()
        );
    }

    tapeSize_ = static_cast<std::size_t>(fileSizeInBytes / sizeof(ValueType));
}

void FileTape::createEmptyFile(std::size_t cellCount) {
    // Получение папки, в которой должен быть создан файл ленты.
    const std::filesystem::path parentPath = filePath_.parent_path();

    // Создание папки для файла, если путь к папке указан.
    if (!parentPath.empty()) {
        std::filesystem::create_directories(parentPath);
    }

    // Создание нового бинарного файла ленты.
    // Если файл уже существует, его содержимое очищается.
    std::ofstream output(filePath_, std::ios::binary | std::ios::trunc);

    if (!output.is_open()) {
        throw std::runtime_error("Cannot create tape file: " + filePath_.string());
    }

    // Пустая ячейка ленты.
    const ValueType zero = 0;

    // Заполнение новой ленты нулями, чтобы файл сразу имел нужный размер.
    for (std::size_t index = 0; index < cellCount; ++index) {
        output.write(
            reinterpret_cast<const char*>(&zero), // превращает адрес числа в указатель на байты.
            sizeof(ValueType)
        );
    }

    if (output.fail()) {
        throw std::runtime_error("Cannot initialize tape file: " + filePath_.string());
    }

    // Сохранение количества ячеек созданной ленты.
    tapeSize_ = cellCount;
}

ITape::ValueType FileTape::read() {
    wait(config_.readDelayMs);

    if (isEnd()) {
        throw std::runtime_error("Cannot read from the end of tape");
    }

    ValueType value = 0;

    file_.clear();
    file_.seekg(currentByteOffset(), std::ios::beg);
    file_.read(reinterpret_cast<char*>(&value), sizeof(ValueType));

    if (file_.fail()) {
        throw std::runtime_error("Cannot read tape cell");
    }

    return value;
}

void FileTape::write(ValueType value) {
    wait(config_.writeDelayMs);

    if (isEnd()) {
        throw std::runtime_error("Cannot write to the end of tape");
    }

    file_.clear();
    file_.seekp(currentByteOffset(), std::ios::beg);
    file_.write(reinterpret_cast<const char*>(&value), sizeof(ValueType));
    file_.flush();

    if (file_.fail()) {
        throw std::runtime_error("Cannot write tape cell");
    }
}

void FileTape::moveLeft() {
    wait(config_.moveDelayMs);

    if (currentPosition_ == 0) {
        throw std::runtime_error("Cannot move left from the beginning of tape");
    }

    --currentPosition_;
}

void FileTape::moveRight() {
    wait(config_.moveDelayMs);

    if (currentPosition_ >= tapeSize_) {
        throw std::runtime_error("Cannot move right from the end of tape");
    }

    ++currentPosition_;
}

void FileTape::rewindToBegin() {
    wait(config_.rewindDelayMs);
    currentPosition_ = 0;
}

void FileTape::rewindToEnd() {
    wait(config_.rewindDelayMs);

    if (tapeSize_ == 0) {
        currentPosition_ = 0;
    }
    else {
        currentPosition_ = tapeSize_ - 1;
    }
}

bool FileTape::isAtBegin() const {
    return currentPosition_ == 0;
}

bool FileTape::isEnd() const {
    return currentPosition_ >= tapeSize_;
}

std::size_t FileTape::position() const {
    return currentPosition_;
}

std::size_t FileTape::size() const {
    return tapeSize_;
}

void FileTape::wait(std::uint64_t delayMs) const {
    if (delayMs == 0) {
        return;
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(delayMs)
    );
}

// currentByteOffset() возвращает байтовое смещение текущей ячейки в файле.
// Тип std::streamoff подходит именно для смещений файлового потока.
std::streamoff FileTape::currentByteOffset() const {
    return static_cast<std::streamoff>(currentPosition_ * sizeof(ValueType));
}