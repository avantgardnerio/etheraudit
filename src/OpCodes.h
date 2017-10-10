#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <assert.h>

namespace OpCodes {
    struct OpCode {
        uint8_t opCode;
        std::string name;
        size_t stackRemoved, stackAdded, length;

        OpCode(uint8_t opMode, const std::string &name, size_t stackRemoved = 0,
               size_t stackAdded = 0, size_t length = 0);

        OpCode& operator=(const OpCode&) = delete;
        OpCode(const OpCode&) = delete;

        bool isFallThrough() const;
        bool isBranch() const;
        bool isUnknown() const;
        int dupNum() const;
        int swapNum() const;
        int pushNum() const;

        std::string Infix() const;

        bool isStop() const;
        bool isStackManipulatorOnly() const;
        bool isArithmetic() const;

        int64_t Solve(const std::vector<int64_t>& input) const;

        bool operator==(const OpCode &rhs) const;

        bool operator!=(const OpCode &rhs) const;
    };

#define XX(NAME, OPCODE, STACKREQ, STACKADD, BYTE_LENGTH) \
    extern const OpCode NAME;\
    static const uint8_t OP_ ## NAME = OPCODE;
#include "opcodes_xx.h"

    extern const OpCode UNKNOWN;

    const OpCode& get(uint8_t opCode);

    template <typename F>
    void iterate(const std::vector<uint8_t>& bc, F f) {
        const uint8_t* data = bc.data();
        while(data < bc.data() + bc.size()) {
            auto pos = data - bc.data();
            auto& opCode = get(*data);
            if(data + opCode.length < bc.data() + bc.size()) {
                f(data + 1, pos, opCode);
            }
            data += opCode.length + 1;
        }
    }

}
