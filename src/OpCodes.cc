#include "OpCodes.h"
#include <iostream>
#include <cmath>
#include "assert.h"


#define XX(NAME, OPCODE, STACKREQ, STACKADD, BYTE_LENGTH) \
const OpCodes::OpCode OpCodes::NAME(OPCODE, #NAME, STACKREQ, STACKADD, BYTE_LENGTH);
#include "opcodes_xx.h"

const OpCodes::OpCode& OpCodes::get(uint8_t opCode) {
    switch(opCode) {
#define XX(NAME, OPCODE, STACKREQ, STACKADD, BYTE_LENGTH) \
case OPCODE: { \
return NAME;\
break;\
}
#include "opcodes_xx.h"
        default: {
            std::cerr << "Unknown opcode: 0x" << std::ios::hex << opCode << std::endl;
            assert(false);
        }
    }

}

OpCodes::OpCode::OpCode(uint8_t opMode, const std::string &name, size_t stackRemoved, size_t stackAdded, size_t length)
        : opCode(
        opMode), name(name), stackRemoved(stackRemoved), stackAdded(stackAdded), length(length) {}

bool OpCodes::OpCode::isBranch() const {
    return opCode == OpCodes::JUMP.opCode ||
           opCode == OpCodes::JUMPI.opCode;
}

bool OpCodes::OpCode::operator==(const OpCodes::OpCode &rhs) const {
    return opCode == rhs.opCode;
}

bool OpCodes::OpCode::operator!=(const OpCodes::OpCode &rhs) const {
    return !(rhs == *this);
}

static int classNum(uint8_t opCode, const OpCodes::OpCode& start,
                    const OpCodes::OpCode& end) {
    if(opCode >= start.opCode &&
       opCode <= end.opCode)
        return opCode - start.opCode;
    return -1;
}

int OpCodes::OpCode::dupNum() const {
    return classNum(opCode, OpCodes::DUP1, OpCodes::DUP16);
}

int OpCodes::OpCode::swapNum() const {
    return classNum(opCode, OpCodes::SWAP1, OpCodes::SWAP16);
}

bool OpCodes::OpCode::isArithmetic() const {
    if(opCode >= OpCodes::OP_ADD && opCode < OpCodes::OP_SHA3)
        return true;

    return false;
}

static int64_t iexp(int64_t a, int64_t b) {
    if(b == 0)
        return 1;
    if(b == 1)
        return a;

    auto hb = b / 2;

    auto rtn = iexp(a, hb);
    if(b % 1)
        rtn *= a;
    return rtn;
}

int64_t OpCodes::OpCode::Solve(const std::vector<int64_t> &input) const {
    assert(isArithmetic());
    switch(opCode) {
        case OP_EXP:
            assert(input.size() == 2);
            return iexp(input[0], input[1]);
        case OP_DIV:
            assert(input.size() == 2);
            if(input[1] == 0)
                return 0;
            return input[0] / input[1];
        case OP_NOT:
            assert(input.size() == 1);
            return ~input[0];
        case OP_SUB:
            assert(input.size() == 2);
            return input[0] - input[1];
        case OP_AND:
            assert(input.size() == 2);
            return input[0] & input[1];
        case OP_ADD:
            assert(input.size() == 2);
            return input[0] + input[1];
        default:
            assert(false && "Please add the logic!");
    }
    return 0;
}
