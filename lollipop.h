#ifndef LOLLIPOP_HEADER
#define LOLLIPOP_HEADER

// If an OS it created make it so that LLVM code is compiled to this
// Compile GNU softare using the LLVM compiler to compile it
// Use linux or custom kernel if needed
// Write drivers for custom real or virtual components
// For programs not only should it shift all addresses so that it can only write to allocate mem
// But it should also put smart breakpoints for safety reasons among other things

#include <cstddef>
#include <string>
#include <array>
#include <unordered_map>
#include <utility>
//#include <pair>

namespace Lollipop {
    // Header:
    // (header_size) (data)
    // Code:
    // (code_size) (code)

    // Booleans have all bits set to their coressponding boolean
    // Lines starts from 1 and the program ends upon movement to an invalid line unless it's 0, where it'll just cancel
    const size_t numInstructions = 16;
    const size_t maxNumParams = 2;
    enum InstructionType {
        // Gates
        AND, // <target> <toAND>
        OR, // <target> <toOR>
        XOR, // <target> <toXOR>
        NOT, // <target>
        // Bitshifts
        SHIFT, // <target> <amount>
        // Integer operations
        ADD, // <target> <toADD>
        SUB, // <target> <toSUB>
        MUL, // <target> <toMUL>
        DIV, // <target> <toDIV>
        MOD, // <target> <toMOD>
        // Comparisons (these always write to 0)
        LESS, // <param1> <param2>
        EQU, // <param1> <param2>
        // Copying/Derferencing
        COPY, // <addressToAddressTarget> <addressToAddressFrom>
        // Goto
        GOTO, // <line>
        // Taking input
        INPUT, // <target>
        // Loading assembly dynamically
        LOAD // <target> <code>
    };

    struct InstructionData {
        std::string str;
        size_t numParams;

        InstructionData(std::string str, size_t numParams) {
            this->str = str;
            this->numParams = numParams;
        }
    };

    // InstructionType to InstructionData
    const std::array<InstructionData, numInstructions> instructionData = {
        InstructionData("AND", 2),
        InstructionData("OR", 2),
        InstructionData("XOR", 2),
        InstructionData("NOT", 2),
        InstructionData("SHIFT", 2),
        InstructionData("ADD", 2),
        InstructionData("SUB", 2),
        InstructionData("MUL", 2),
        InstructionData("DIV", 2),
        InstructionData("MOD", 2),
        InstructionData("LESS", 2),
        InstructionData("EQU", 2),
        InstructionData("COPY", 2),
        InstructionData("GOTO", 1),
        InstructionData("INPUT", 1),
        InstructionData("LOAD", 2)
    };

    //#define StrInstructionPair(ins) { instructionData[ins].str, ins }
    std::pair<std::string, InstructionType> StrInstructionPair(InstructionType ins) {
        return { instructionData[ins].str, ins };
    }

    // std::string to InstructionType
    const std::unordered_map<std::string, InstructionType> strToIns = {
        StrInstructionPair(InstructionType::AND),
        StrInstructionPair(InstructionType::OR),
        StrInstructionPair(InstructionType::XOR),
        StrInstructionPair(InstructionType::NOT),
        StrInstructionPair(InstructionType::SHIFT),
        StrInstructionPair(InstructionType::ADD),
        StrInstructionPair(InstructionType::SUB),
        StrInstructionPair(InstructionType::MUL),
        StrInstructionPair(InstructionType::DIV),
        StrInstructionPair(InstructionType::MOD),
        StrInstructionPair(InstructionType::LESS),
        StrInstructionPair(InstructionType::EQU),
        StrInstructionPair(InstructionType::COPY),
        StrInstructionPair(InstructionType::GOTO),
        StrInstructionPair(InstructionType::INPUT),
        StrInstructionPair(InstructionType::LOAD)
    };

    // NBit means n bits which means that the type represents the amount of bits used

    template <typename NBit>
    struct Instruction {
        // The type of instruction
        InstructionType type;
        // The parameters
        std::array<NBit, maxNumParams> params;

        Instruction(InstructionType type, std::array<NBit, maxNumParams> params = std::array<NBit, maxNumParams>()) {
            this->type = type;
            this->params = params;
        }

        std::array<char, 1 + sizeof(NBit) * maxNumParams> compressed_bytes() {
            std::array<char, 1 + sizeof(NBit) * maxNumParams> data =
                std::array<char, 1 + sizeof(NBit) * maxNumParams>();
            data[0] = (char)type << 4; // 4 bits
            for (size_t i = 0; i < params.size(); i++)
                for (size_t j = 0; j < sizeof(NBit); j++) {
                    data[i] |= (maxNumParams >> 4); // flipped?
                    data[i + 1] = maxNumParams << 4;
                }
            return data;
        }

        std::string to_string() {
            std::string toReturn = instructionData[this->type].str;
            for (NBit param : params)
                toReturn += " " + std::to_string(param);
            return toReturn;
        }
    };

    enum EndReason {
        Null,
        Natural,
        Input,
        Error
    };

    template <typename NBit>
    class Interpreter {
    public:
        // The bytecode
        Instruction<NBit>* byteCode;
        // The memory
        NBit* memory;
        // The current line
        NBit line;

        Interpreter() {

        }

        // This will run until the program ends, an exception happens, or an input statement is reached
        EndReason run() {
            return EndReason::Natural;
        }

        // This will run a tick of the program
        EndReason run_tick() {
            return EndReason::Null;
        }

        // This is what is used to enter an input
        void input() {
            
        }

        // This creates a new instance of the memory and outputs it
        void mem_dump() {

        }

        // This will return the value at an address
        NBit get_addr() {
            
        }

        // This will set the value at an address
        void set_addr() {

        }
    };
}

#endif
