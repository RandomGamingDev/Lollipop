#ifndef LOLLIPOP_HEADER
#define LOLLIPOP_HEADER

// If an OS it created make it so that LLVM code is compiled to this
// Compile GNU softare using the LLVM compiler to compile it
// Use linux or custom kernel if needed
// Write drivers for custom real or virtual components
// For programs not only should it shift all addresses so that it can only write to allocate mem
// But it should also put smart breakpoints for safety reasons among other things

#include <iostream>

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

    template <typename NBit>
    struct InstructionData {
        std::string str;
        size_t numParams;
        void (*op)(NBit*, NBit*, NBit&);

        InstructionData(std::string str, size_t numParams, void (*op)(NBit*, NBit*, NBit&)) {
            this->str = str;
            this->numParams = numParams;
            this->op = op;
        }
    };

    #define INSTRUCTION_OP(instruction) \
        [](uint64_t* mem, uint64_t* args, uint64_t& i){ instruction; }

    // InstructionType to InstructionData
    const std::array<InstructionData<uint64_t>, numInstructions> instructionData = {
        InstructionData<uint64_t>("AND", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[0]] & mem[args[1]])),
        InstructionData<uint64_t>("OR", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[0]] | mem[args[1]])),
        InstructionData<uint64_t>("XOR", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[0]] ^ mem[args[1]])),
        InstructionData<uint64_t>("NOT", 2, INSTRUCTION_OP(mem[args[0]] = ~mem[args[0]])),
        InstructionData<uint64_t>("SHIFT", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[1]] > 0 ? mem[args[0]] >> mem[args[1]] : mem[args[0]] << -mem[args[1]])),
        InstructionData<uint64_t>("ADD", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[0]] + mem[args[1]])),
        InstructionData<uint64_t>("SUB", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[0]] - mem[args[1]])),
        InstructionData<uint64_t>("MUL", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[0]] * mem[args[1]])),
        InstructionData<uint64_t>("DIV", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[0]] / mem[args[1]])),
        InstructionData<uint64_t>("MOD", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[0]] % mem[args[1]])),
        InstructionData<uint64_t>("LESS", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[0]] < mem[args[1]])),
        InstructionData<uint64_t>("EQU", 2, INSTRUCTION_OP(mem[args[0]] = mem[args[0]] == mem[args[1]])),
        InstructionData<uint64_t>("COPY", 2, INSTRUCTION_OP(mem[args[1]] = mem[args[0]])),
        InstructionData<uint64_t>("GOTO", 1, INSTRUCTION_OP(i = mem[args[0]])),
        InstructionData<uint64_t>("INPUT", 1, INSTRUCTION_OP(i = mem[args[0]])),
        InstructionData<uint64_t>("LOAD", 2, INSTRUCTION_OP(i = mem[args[0]]))
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

        std::array<uint8_t, 1 + sizeof(NBit) * maxNumParams> bytes() {
            std::array<uint8_t, 1 + sizeof(NBit) * maxNumParams> data =
                std::array<uint8_t, 1 + sizeof(NBit) * maxNumParams>();
            
            data[0] = static_cast<uint8_t>(this->type);
            // 
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
        EndReason run() {
            while (this->endReason == EndReason::Null)
                this->run_tick();
            return this->endReason;
        }

        // This will run a tick of the program
        EndReason run_tick() {
            Instruction<NBit> instruction = byteCode[line - 1];
            Lollipop::InstructionData<uint64_t> instructionData = Lollipop::instructionData[instruction.type];

            std::cout << instructionData.str << std::endl;
            instructionData.op(this->memory, instruction.params.data(), this->line);
            std::cout << this->memory[0] << std::endl;
            std::cout << this->memory[1] << std::endl;
            std::cout << this->memory[2] << std::endl;

            this->line++;
            if (this->line > byteCodeSize)
                this->endReason = EndReason::Natural;
            else
                this->endReason = EndReason::Null;

            return this->endReason;
        }

        // This is what is used to enter an input
        void input() {
            
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