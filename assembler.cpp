#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdint>
#include <vector>
#include <ctype.h>
#include <cstddef>
#include <optional>

#include "lollipop.h"

const std::string indent = "  ";

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

template <typename T>
std::optional<T> str_to_uint(std::string str) {
    T toReturn = 0;
    T place = 1;

    // The integer will overflow and the 2nd part will detect that
    for (size_t i = str.length() - 1; i < str.length(); i--) {
        const char digit = str[i] - 48;

        // Return error if it isn't a number
        if (digit > 9)
            return std::nullopt; // change this to optional later

        toReturn += digit * place;
        place *= 10;
    }

    return toReturn;
}

int main(int argc, char* argv[]) {
    // Get the file path
    const std::string toAssemblePath = 
        (argc < 2) ? 
            input("Enter the file that you'd like to assemble: ") :
            argv[1];

    // Get the file
    std::ifstream asmFile(toAssemblePath, std::ios::in);
    if (!asmFile.is_open())
        end_with_error("Failed to open " << toAssemblePath);

    // Line data
    std::string line;
    size_t lineI = 0;

    { // Read the header's header
        getline(asmFile, line);
        lineI++;
        if (line != "header {")
            end_with_error("The header is missing!");
    }

    // Read the header data
    std::vector<uint64_t> headerData;
    while (getline(asmFile, line)) {
        // Check whether the header's ended
        if (line == "}")
            break;

        // Check whether the indentation was done properly
        if (line.length() < 3 || line.substr(0, indent.length()) != indent) // 2 spaces
            end_with_error("Improper indentation in the header on line " << (lineI + 1));

        const size_t skipInd = indent.length();
        const std::string dataStr = line.substr(skipInd, line.length() - skipInd);
        // Get the value
        const std::optional<uint64_t> res = str_to_uint<uint64_t>(dataStr);
        if (!res.has_value())
            end_with_error("Failed to parse line " << (lineI + 1));
        headerData.push_back(res.value());

        lineI++;
    }
    lineI++;

    // Read the code
    std::vector<Lollipop::Instruction<uint64_t>> instructions;
    while (getline(asmFile, line)) {
        if (line[0] == '#')
            continue;
        if (line.length() == 0)
            continue;

        // Start & End Cursors
        size_t sCursor = 0;
        size_t eCursor = 0;

        { // Get the command
            // Moving the end cursor to the end of the command
            for (;eCursor < line.length() && line[eCursor] != ' '; eCursor++);

            // Getting the command
            const std::string cmd = line.substr(sCursor, eCursor);
            const auto cmdResult = Lollipop::strToIns.find(cmd);

            // Check whether it's a valid command
            if (cmdResult == Lollipop::strToIns.end())
                end_with_error("There's an invalid command on line " << (lineI + 1));
            
            // The instruction data
            const Lollipop::InstructionType instructionType = cmdResult->second;
            const Lollipop::InstructionData instructionData = Lollipop::instructionData[instructionType];
            const size_t numParams = instructionData.numParams;

            // The instruction
            Lollipop::Instruction<uint64_t> instruction = Lollipop::Instruction<uint64_t>(instructionType);

            // Loop over and get the parameters
            for (size_t i = 0; i < numParams; i++) {
                // move the starting cursor to where the end cursor is
                eCursor++;
                sCursor = eCursor;
                // Moving the end cursor to the end of the parameter
                for (;eCursor < line.length() && line[eCursor] != ' '; eCursor++);

                // Getting the parameter
                const std::string paramStr = line.substr(sCursor, eCursor - sCursor);
                const std::optional<uint64_t> res = str_to_uint<uint64_t>(paramStr);
                if (!res.has_value())
                    end_with_error("There's an invalid parameter on line " << (lineI + 1) << " for parameter " << (i + 1));
                instruction.params[i] = res.value();
            }

            // Add the instruction to the instructions
            instructions.push_back(instruction); 
        }

        lineI++;
    }

    asmFile.close();

    // Create and store a binary file containing the bytecode
    std::string name = 
        argc < 3 ?
            input("Enter the output location for the bytecode file: ") :
            argv[2];

    // Initialize the binary file
    std::ofstream byteFile(name, std::ios::out | std::ios::binary);
    if (!byteFile)
        end_with_error("Failed to open " << name);

    // Write all of the binary data

    { // Write the header data
        // Write the header's size
        uint64_t headerSize = static_cast<uint64_t>(headerData.size());
        byteFile.write(reinterpret_cast<char*>(&headerSize), sizeof(uint64_t));
        // Write the header data
        byteFile.write(reinterpret_cast<char*>(headerData.data()), headerData.size() * sizeof(uint64_t));
    }

    { // Write the bytecode
        for (Lollipop::Instruction<uint64_t>& instruction : instructions) {
            std::array<uint8_t, 1 + sizeof(uint64_t) * Lollipop::maxNumParams> bytes = instruction.bytes();
            byteFile.write(reinterpret_cast<char*>(bytes.data()), bytes.size() * sizeof(uint8_t));
        }
    }
    
    // Close & make sure that everything went right while writing
    byteFile.close();
    if (!byteFile.good())
        end_with_error("Something went wrong while writing to " << name);
}