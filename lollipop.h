#ifndef LOLLIPOP_HEADER
#define LOLLIPOP_HEADER

// Make it so that LLVM code is compiled to this (maybe?)
// Write drivers for custom real or virtual components
// For programs not only should it shift all addresses so that it can only write to allocate mem

#include <iostream>

#include <optional>
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

    template <typename NBit>
    class Executor;

    uint64_t input() {
        // Get the input from console
        std::string input;
        std::getline(std::cin, input);

        // Get the value
        const std::optional<uint64_t> op_val = str_to_uint<uint64_t>(input);
        // If it's invalid take NULL
        if (!op_val.has_value())
            return 0;

        // Otherwise continue on with letting the program take in the input
        return op_val.value();
    }

    template <typename NBit>
    struct InstructionData {
        std::string str;
        size_t numParams;
        void (*op)(Executor<uint64_t>*, NBit*, NBit*, NBit&);

        InstructionData(std::string str, size_t numParams, void (*op)(Executor<uint64_t>*, NBit*, NBit*, NBit&)) {
            this->str = str;
            this->numParams = numParams;
            this->op = op;
        }
    };

    #define INSTRUCTION_OP(instruction) [](Executor<uint64_t>* exec, uint64_t* mem, uint64_t* args, uint64_t& line){ instruction; }
    #define INSTRUCTION_DATA(str, numParams, op) InstructionData<uint64_t>(str, numParams, INSTRUCTION_OP(op))

    // Argument Memory Reference
    #define AMR(i) mem[args[i]]

    // InstructionType to InstructionData
    const std::array<InstructionData<uint64_t>, numInstructions> instructionData = {
        INSTRUCTION_DATA("AND", 2, AMR(0) &= AMR(1)),
        INSTRUCTION_DATA("OR", 2, AMR(0) |= AMR(1)),
        INSTRUCTION_DATA("XOR", 2, AMR(0) ^= AMR(1)),
        INSTRUCTION_DATA("NOT", 2, AMR(0) = ~AMR(0)),
        INSTRUCTION_DATA("SHIFT", 2, AMR(0) = AMR(1) > 0 ? AMR(0) >> AMR(1) : AMR(0) << -AMR(1)),
        INSTRUCTION_DATA("ADD", 2, AMR(0) += AMR(1)),
        INSTRUCTION_DATA("SUB", 2, AMR(0) -= AMR(1)),
        INSTRUCTION_DATA("MUL", 2, AMR(0) *= AMR(1)),
        INSTRUCTION_DATA("DIV", 2, AMR(0) /= AMR(1)),
        INSTRUCTION_DATA("MOD", 2, AMR(0) %= AMR(1)),
        INSTRUCTION_DATA("LESS", 2, AMR(0) = AMR(0) < AMR(1)),
        INSTRUCTION_DATA("EQU", 2, AMR(0) = AMR(0) == AMR(1)),
        INSTRUCTION_DATA("COPY", 2, AMR(1) = AMR(0)),
        INSTRUCTION_DATA("GOTO", 1, line = AMR(0)),
        INSTRUCTION_DATA("INPUT", 1, AMR(0) = input()),
        INSTRUCTION_DATA("LOAD", 2, AMR(0) = mem[AMR(1)])
        /*
        InstructionData<uint64_t>("AND", 2, INSTRUCTION_OP(AMR(0) &= AMR(1))),
        InstructionData<uint64_t>("OR", 2, INSTRUCTION_OP(AMR(0) |= AMR(1))),
        InstructionData<uint64_t>("XOR", 2, INSTRUCTION_OP(AMR(0) ^= AMR(1))),
        InstructionData<uint64_t>("NOT", 2, INSTRUCTION_OP(AMR(0) = ~AMR(0))),
        InstructionData<uint64_t>("SHIFT", 2, INSTRUCTION_OP(AMR(0) = AMR(1) > 0 ? AMR(0) >> AMR(1) : AMR(0) << -AMR(1))),
        InstructionData<uint64_t>("ADD", 2, INSTRUCTION_OP(AMR(0) += AMR(1))),
        InstructionData<uint64_t>("SUB", 2, INSTRUCTION_OP(AMR(0) -= AMR(1))),
        InstructionData<uint64_t>("MUL", 2, INSTRUCTION_OP(AMR(0) *= AMR(1))),
        InstructionData<uint64_t>("DIV", 2, INSTRUCTION_OP(AMR(0) /= AMR(1))),
        InstructionData<uint64_t>("MOD", 2, INSTRUCTION_OP(AMR(0) %= AMR(1))),
        InstructionData<uint64_t>("LESS", 2, INSTRUCTION_OP(AMR(0) = AMR(0) < AMR(1))),
        InstructionData<uint64_t>("EQU", 2, INSTRUCTION_OP(AMR(0) = AMR(0) == AMR(1))),
        InstructionData<uint64_t>("COPY", 2, INSTRUCTION_OP(AMR(1) = AMR(0))),
        InstructionData<uint64_t>("GOTO", 1, INSTRUCTION_OP(line = AMR(0))),
        InstructionData<uint64_t>("INPUT", 1, INSTRUCTION_OP(AMR(0) = input())),
        InstructionData<uint64_t>("LOAD", 2, INSTRUCTION_OP(AMR(0) = mem[AMR(1)]))
        */
    };

    #undef AMR

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

        std::array<uint8_t, 1 + sizeof(NBit) * maxNumParams> bytes() {
            std::array<uint8_t, 1 + sizeof(NBit) * maxNumParams> data =
                std::array<uint8_t, 1 + sizeof(NBit) * maxNumParams>();
            
            data[0] = static_cast<uint8_t>(this->type);
            std::copy(this->params.begin(), this->params.end(), reinterpret_cast<uint64_t*>(&data[1]));

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
    class Executor {
    public:
        // The bytecode
        Instruction<NBit>* byteCode;
        NBit byteCodeSize;
        // The memory
        NBit* memory;
        NBit memorySize;
        // The current line
        NBit line;
        // EndReason
        EndReason endReason;

        Executor(
            Instruction<NBit>* byteCode,
            NBit byteCodeSize,
            NBit* memory,
            NBit memorySize,
            NBit line = 1,
            EndReason endReason = EndReason::Null
        ) {
            this->byteCode = byteCode;
            this->byteCodeSize = byteCodeSize;
            this->memory = memory;
            this->memorySize = memorySize;
            this->line = line;
            this->endReason = endReason;
        }

        // This will run until the program ends, an exception happens, or an input statement is reached
        EndReason run(void (*callback)(Executor<NBit>*) = nullptr) {
            while (this->endReason == EndReason::Null) {
                this->run_tick();
                if (callback != nullptr)
                    callback(this);
            }
            return this->endReason;
        }

        // This will run a tick of the program
        EndReason run_tick() {
            Instruction<NBit> instruction = byteCode[line - 1];
            Lollipop::InstructionData<uint64_t> instructionData = Lollipop::instructionData[instruction.type];

            instructionData.op(this, this->memory, instruction.params.data(), this->line);

            this->line++;
            if (this->line > byteCodeSize)
                this->endReason = EndReason::Natural;
            else
                this->endReason = EndReason::Null;

            return this->endReason;
        }

        // This creates a new instance of the memory and outputs it
        NBit* mem_dump() {
            NBit* to_return = new NBit[sizeof(this->memory) / sizeof(NBit)];
            std::copy(this->memory, this->memory + sizeof(this->memory), to_return);
            return to_return;
        }

        // This will return the value at an address
        NBit get_addr(NBit i) {
            if (i < 0 || i >= sizeof(memory) / sizeof(NBit)) {
                this->endReason = EndReason::Error;
                return NULL;
            }
            return this->memory[i];
        }

        // This will set the value at an address
        void set_addr(NBit i, NBit val) {
            if (i < 0 || i >= sizeof(memory) / sizeof(NBit)) {
                this->endReason = EndReason::Error;
                return;
            }
            this->memory[i] = val;
        }

        // Check to make sure goto is safe
        bool check_goto(NBit i) {
            return false;
        }
    };
}

#endif