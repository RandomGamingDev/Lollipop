#include <iostream>
#include <string>
#include <cstddef>
#include <fstream>
#include <vector>

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
    const std::string toDisassemblePath = 
        (argc < 2) ? 
            input("Enter the file that you'd like to disassemble: ") :
            argv[1];
            
    // Get the file
    std::ifstream byteFile(toDisassemblePath, std::ios::in | std::ios::binary);
    if (!byteFile.is_open())
        end_with_error("Failed to open " << toDisassemblePath);

    std::vector<unsigned char> byteVector(std::istreambuf_iterator<char>(byteFile), {});
    size_t byteBufferSize = byteVector.size() * sizeof(unsigned char);
    uint8_t* byteBuffer = byteVector.data();

    std::string asmString = "header {\n";

    // Read the header
    uint64_t byteHeaderSize = *reinterpret_cast<uint64_t*>(&byteBuffer[0]);
    uint64_t endOfHeader = (byteHeaderSize + 1) * sizeof(uint64_t);
    // Read all header values except for the header length
    for (size_t i = sizeof(byteHeaderSize); i < endOfHeader; i += sizeof(uint64_t))
        asmString += "  " + std::to_string(*reinterpret_cast<uint64_t*>(&byteBuffer[i])) + "\n";
    asmString += "}\n";

    // Read the instructions
    for (size_t i = endOfHeader; i < byteBufferSize;) {
        // Read the instruction and get its meta data
        Lollipop::InstructionType instruction = static_cast<Lollipop::InstructionType>(byteBuffer[i]);
        Lollipop::InstructionData instructionData = Lollipop::instructionData[instruction];
        asmString += instructionData.str;

        // Start reading the parameters
        i += sizeof(unsigned char);
        size_t instructionEnd = i + instructionData.numParams * sizeof(uint64_t);
        for (;i < instructionEnd; i += sizeof(uint64_t))
            asmString += " " + std::to_string(*reinterpret_cast<uint64_t*>(&byteBuffer[i]));
        
        // End the line
        asmString += "\n";
    }

    std::cout << asmString;
}