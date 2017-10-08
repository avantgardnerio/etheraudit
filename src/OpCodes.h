#pragma once

#include <stdlib.h>
#include <string>
#include <vector>

namespace OpCodes {
    struct OpCode {
        uint8_t opCode;
        std::string name;
        size_t stackRemoved, stackAdded, length;

        OpCode(uint8_t opMode, const std::string &name, size_t stackRemoved, size_t stackAdded, size_t length);

        OpCode& operator=(const OpCode&) = delete;
        OpCode(const OpCode&) = delete;

        bool isBranch() const;
        int dupNum() const;
        int swapNum() const;
        int pushNum() const;
        bool isStackManipulatorOnly() const;

        int64_t Solve(const std::vector<int64_t>& input) const;
        bool isArithmetic() const;

        bool operator==(const OpCode &rhs) const;

        bool operator!=(const OpCode &rhs) const;
    };

#define XX(NAME, OPCODE, STACKREQ, STACKADD, BYTE_LENGTH) \
    extern const OpCode NAME;\
    static const uint8_t OP_ ## NAME = OPCODE;
#include "opcodes_xx.h"

    const OpCode& get(uint8_t opCode);

    template <typename F>
    void iterate(const std::vector<uint8_t>& bc, F f) {
        const uint8_t* data = bc.data();
        while(data != bc.data() + bc.size()) {
            auto pos = data - bc.data();
            auto& opCode = get(*data);
            f(data + 1, pos, opCode);
            data += opCode.length + 1;
        }
    }

}
