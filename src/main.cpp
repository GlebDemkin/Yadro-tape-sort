#include "FileTape.h"
#include "TapeConfig.h"
#include "TapeSorter.h"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        if (argc < 3) {
            std::cerr << "Usage: yadro_tape_sort <input_file> <output_file> [config_file]\n";
            return 1;
        }

        const std::string inputFilePath = argv[1];
        const std::string outputFilePath = argv[2];

        std::string configFilePath = "config.txt";
        if (argc >= 4) {
            configFilePath = argv[3];
        }

        const TapeConfig config =
            TapeConfig::loadFromFile(configFilePath);

        FileTape inputTape(
            inputFilePath,
            config,
            FileTape::OpenMode::ReadWriteExisting
        );

        FileTape outputTape(
            outputFilePath,
            config,
            FileTape::OpenMode::CreateNew,
            inputTape.size()
        );

        TapeSorter sorter(config, "tmp");

        sorter.sort(inputTape, outputTape);

        std::cout << "Sorting completed successfully.\n";

        return 0;
    }
    catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }
}