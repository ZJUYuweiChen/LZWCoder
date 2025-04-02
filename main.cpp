#include <iostream>
#include "LZWCoder.hpp"

std::tuple<std::string, std::string> splitFileName(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == filePath.length() - 1) {
        return std::make_tuple(filePath, "");
    }
    std::string filename = filePath.substr(0, dotPos);
    std::string extension = filePath.substr(dotPos + 1);
    return {filename, extension};
}

int main(int argc, char* argv[]) {
    LZWCoder coder;
    if (argc == 2) {
        auto [filename, extension] = splitFileName(argv[1]);
        if (extension == "txt") { coder.encode(filename + ".lzw", filename + ".txt"); }
        else if (extension == "lzw") { coder.encode(filename + ".txt", filename + ".lzw"); }
        else { std::cout << "Invalid arguments!" << std::endl; }
    } else if (argc == 4) {
        if (std::string(argv[2]) == "-e") { coder.encode(argv[3], argv[1]); }
        else if (std::string(argv[2]) == "-d") { coder.decode(argv[3], argv[1]); }
        else { std::cout << "Invalid arguments!" << std::endl; }
    } else {
        std::cout << "Invalid arguments!" << std::endl;
    }
    return 0;
}