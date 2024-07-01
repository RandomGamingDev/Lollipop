#ifndef LOLLIPOP_HEADER
#define LOLLIPOP_HEADER

// Add a way to specify memory required in header
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
#include <type_traits>
#define FMT_HEADER_ONLY
#include <fmt/core.h> // sudo apt install libfmt-dev

namespace Lollipop {
    // Utility Functions
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

    uint64_t input() {
        // Flush input buffer
        std::cin.clear();
        std::cin.sync();

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

    // Forward Declared Classes
    template <typename NBit>
    class Executor;

    // Enums and consts

    // Booleans have all bits set to their coressponding boolean
    // Lines starts from 1 and the program ends upon movement to an invalid line unless it's 0, where it'll just cancel
    const size_t NUM_INSTRUCTIONS = 16;
    const size_t MAX_NUM_PARAMS = 2;
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

    enum EndReason {
        Null, // When there's no end reason
        Natural, // When the program ends without error
        Input, // When the program stops to receive input
        Error // When the program crashes due to error
    };

    // Class Declarations

    template <typename NBit>
    class Memory {
    public:
        NBit* array;
        NBit size;

        Memory(NBit* array, NBit size) {
            static_assert(std::is_unsigned_v<NBit> == true);

            this->array = array;
            this->size = size;
        }

        // For safely accessing memory
        NBit& operator[](NBit i) {
            // For uint hopefully the latter gets optimized out depending on compiler
            // , but if it doesn't the performance cost shouldn't be too large
            if (i >= size || i < 0)
                throw std::invalid_argument(fmt::format("Index {} is out of bounds: 0 to {} (inclusive to exclusive).", i, this->size));
            return array[i];
        }

        // Clone the memory
        NBit* clone() {
            NBit* new_array = new NBit[sizeof(this->array) / sizeof(NBit)];
            std::copy(this->array, this->array + sizeof(this->array), new_array);

            return Memory(new_array, this->size);
        }
   };

    template <typename NBit>
    struct InstructionData {
        std::string str;
        size_t numParams;
        void (*op)(Memory<NBit>, std::array<NBit, MAX_NUM_PARAMS>, NBit&, EndReason&);

        InstructionData(std::string str, size_t numParams, void (*op)(Memory<NBit>, std::array<NBit, MAX_NUM_PARAMS>, NBit&, EndReason&)) {
            static_assert(std::is_unsigned_v<NBit> == true);

            this->str = str;
            this->numParams = numParams;
            this->op = op;
        }
    };

    #define OP(instruction) [](Memory<uint64_t> mem, std::array<uint64_t, MAX_NUM_PARAMS> args, uint64_t& line, EndReason& endReason){ instruction; }
    #define INS(str, numParams, op) InstructionData<uint64_t>(str, numParams, OP(op))
    #define arg0 args[0]
    #define arg1 args[1]
    #define marg0 mem[arg0]
    #define marg1 mem[arg1]

    // InstructionType to InstructionData
    const std::array<InstructionData<uint64_t>, NUM_INSTRUCTIONS> instructionData = {
        INS("AND", 2, marg0 &= marg1),
        INS("OR", 2, marg0 |= marg1),
        INS("XOR", 2, marg0 ^= marg1),
        INS("NOT", 2, marg0 = ~marg0),
        INS("SHIFT", 2, {
            marg0 = marg1 > 0 ?
                marg0 >> marg1 :
                marg0 << -marg1;
        }),
        INS("ADD", 2, marg0 += marg1),
        INS("SUB", 2, marg0 -= marg1),
        INS("MUL", 2, marg0 *= marg1),
        INS("DIV", 2, marg0 /= marg1),
        INS("MOD", 2, marg0 %= marg1),
        INS("LESS", 2, marg0 = marg0 < marg1),
        INS("EQU", 2, marg0 = marg0 == marg1),
        INS("COPY", 2, marg1 = marg0),
        INS("GOTO", 2, {
            line = arg1;
            // Depending on the first argument jump between references
            for (uint64_t i = 0; i < arg0; i++)
                line = mem[line];

            // End if it's 0
            if (line == 0)
                endReason = EndReason::Natural;

            // Reverse effect of 0 being natural end and line increment at end of execution
            line -= 2;
        }),
        INS("INPUT", 1, {
            endReason = EndReason::Input;
            marg0 = input();
            endReason = EndReason::Null;
        }),
        INS("LOAD", 2, marg0 = mem[marg1])
    };

    #undef INS
    #undef OP
    #undef marg0
    #undef marg1

    #define SIP(ins) { instructionData[ins].str, ins }

    // std::string to InstructionType
    const std::unordered_map<std::string, InstructionType> strToIns = {
        SIP(InstructionType::AND),
        SIP(InstructionType::OR),
        SIP(InstructionType::XOR),
        SIP(InstructionType::NOT),
        SIP(InstructionType::SHIFT),
        SIP(InstructionType::ADD),
        SIP(InstructionType::SUB),
        SIP(InstructionType::MUL),
        SIP(InstructionType::DIV),
        SIP(InstructionType::MOD),
        SIP(InstructionType::LESS),
        SIP(InstructionType::EQU),
        SIP(InstructionType::COPY),
        SIP(InstructionType::GOTO),
        SIP(InstructionType::INPUT),
        SIP(InstructionType::LOAD)
    };

    #undef SIP

    // NBit means n bits which means that the type represents the amount of bits used
    template <typename NBit>
    struct Instruction {
        // The type of instruction
        InstructionType type;
        // The parameters
        std::array<NBit, MAX_NUM_PARAMS> params;

        Instruction(InstructionType type, std::array<NBit, MAX_NUM_PARAMS> params = std::array<NBit, MAX_NUM_PARAMS>()) {
            static_assert(std::is_unsigned_v<NBit> == true);

            this->type = type;
            this->params = params;
        }

        std::array<uint8_t, 1 + sizeof(NBit) * MAX_NUM_PARAMS> bytes() {
            std::array<uint8_t, 1 + sizeof(NBit) * MAX_NUM_PARAMS> data =
                std::array<uint8_t, 1 + sizeof(NBit) * MAX_NUM_PARAMS>();
            
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

    template <typename NBit> // Make sure that this is unsigned
    class Executor {
    public:
        // The bytecode
        Instruction<NBit>* byteCode;
        NBit byteCodeSize;
        // The memory
        Memory<NBit> memory;
        // The current line
        NBit line;
        // EndReason
        EndReason endReason;

        Executor(
            Instruction<NBit>* byteCode,
            NBit byteCodeSize,
            Memory<NBit> memory,
            NBit line = 0,
            EndReason endReason = EndReason::Null
        ) : memory(memory)
        {
            static_assert(std::is_unsigned_v<NBit> == true);

            this->byteCode = byteCode;
            this->byteCodeSize = byteCodeSize;
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
            // Make sure that the line's safe before continuing
            if (!line_safe()) {
                this->endReason = EndReason::Natural;
                return this->endReason;
            }

            // Get the instruction's data
            const Instruction<NBit>& instruction = byteCode[line];
            const Lollipop::InstructionData<uint64_t>& instructionData = Lollipop::instructionData[instruction.type];

            // Execute the instruction and increment
            try {
                instructionData.op(this->memory, instruction.params, this->line, this->endReason);
                this->line++;
            }
            catch (std::exception& e) { // Don't throw any errors of a type that doesn't inherit from std::exception
                this->endReason = EndReason::Error;
                std::cout << e.what() << std::endl;
            }
            catch (...) {
                this->endReason = EndReason::Error;
                std::cout <<
                    "An exception of a type not inheriting from std::exception was thrown!" << 
                    "Please change the thrown exception to inherit from std::exception!" << std::endl;
            }

            return this->endReason;
        }

        // Check to make sure goto is safe
        bool line_safe() {
            return this->line < byteCodeSize;
        }
    };
}

#endif