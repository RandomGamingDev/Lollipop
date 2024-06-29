#include <iostream>
#include <array>
#include <cstddef>
#include <fstream>
#include <vector>
#include <string>

#include "lollipop.h"

std::string input(std::string prompt) {
    std::cout << prompt << std::endl;

    std::string result;
    std::getline(std::cin, result);
    
    return result;
}

#define end_with_error(error) {\
    std::cout << error << std::endl;\
    return 1;\
}

int main(int argc, char* argv[]) {
    // Get the file path
    const std::string executablePath = 
        (argc < 2) ? 
            input("Enter the file that you'd like to execute: ") :
            argv[1];

    // Get the file
    std::ifstream byteFile(executablePath, std::ios::in | std::ios::binary);
    if (!byteFile.is_open())
        end_with_error("Failed to open " + executablePath);

    std::vector<unsigned char> byteVector(std::istreambuf_iterator<char>(byteFile), {});
    size_t byteBufferSize = byteVector.size() * sizeof(unsigned char);
    uint8_t* byteBuffer = byteVector.data();

    // Read the header
    uint64_t* byteHeader = &reinterpret_cast<uint64_t*>(&byteBuffer[0])[1]; // DEAL
    uint64_t byteHeaderSize = *reinterpret_cast<uint64_t*>(&byteBuffer[0]);
    uint64_t endOfHeader = (byteHeaderSize + 1) * sizeof(uint64_t);


    // Read the instructions
    std::vector<Lollipop::Instruction<uint64_t>> instructions = std::vector<Lollipop::Instruction<uint64_t>>();
    for (size_t i = endOfHeader; i < byteBufferSize;) {
        // Read the instruction and get its meta data
        Lollipop::InstructionType instructionType = static_cast<Lollipop::InstructionType>(byteBuffer[i]);
        Lollipop::InstructionData instructionData = Lollipop::instructionData[instructionType];
        
        i += sizeof(unsigned char);
        // Copy to std::array containing instructions
        std::array<uint64_t, 2> params = std::array<uint64_t, 2>();

        size_t instructionEnd = i + 2 * sizeof(uint64_t);
        std::copy(&byteBuffer[i], &byteBuffer[instructionEnd], reinterpret_cast<uint8_t*>(params.data()));
        i = instructionEnd;

        Lollipop::Instruction<uint64_t> instruction = Lollipop::Instruction<uint64_t>(instructionType, params);
        instructions.push_back(instruction);
    }

    Lollipop::Executor executor =
        Lollipop::Executor<uint64_t>(
            instructions.data(), instructions.size(),
            Lollipop::Memory(byteHeader, byteHeaderSize)
        );

    executor.run(
        [](Lollipop::Executor<uint64_t>* executor) {
            std::cout << "Memory[0]: " << executor->memory[0] << std::endl;
            std::cout << "Line: " << executor->line << std::endl;
        }
    );
}