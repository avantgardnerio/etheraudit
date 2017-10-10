#include "OpCodes.h"
#include <iostream>
#include <cmath>
#include <memory>
#include <map>
#include <sstream>
#include <cstring>
#include "assert.h"


#define XX(NAME, OPCODE, STACKREQ, STACKADD, BYTE_LENGTH) \
const OpCodes::OpCode OpCodes::NAME(OPCODE, #NAME, STACKREQ, STACKADD, BYTE_LENGTH);
#include "opcodes_xx.h"

std::map<size_t, std::unique_ptr<OpCodes::OpCode>> unknownOpCodes;

const OpCodes::OpCode& OpCodes::get(uint8_t opCode) {
    switch(opCode) {
#define XX(NAME, OPCODE, STACKREQ, STACKADD, BYTE_LENGTH) \
case OPCODE: { \
return NAME;\
break;\
}
#include "opcodes_xx.h"
        default: {
            if(unknownOpCodes[opCode] == 0) {
                std::stringstream ss;
                ss << "UNKNOWN(";
                ss.width(2);
                ss.fill('0');
                ss << std::hex << (uint32_t)opCode << ")";
                unknownOpCodes[opCode] = std::make_unique<OpCodes::OpCode>(opCode, ss.str());
            }
            return *unknownOpCodes[opCode];
            //std::cerr << "Unknown opcode: 0x" << std::hex << (uint32_t)opCode << std::endl;
            //assert(false);
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

int OpCodes::OpCode::pushNum() const {
    return classNum(opCode, OpCodes::PUSH1, OpCodes::PUSH32);
}

bool OpCodes::OpCode::isArithmetic() const {
    if(isUnknown())
        return false;
    if(opCode == OP_EXP)
        return false; // It will overflow
    if(opCode == OP_SIGNEXTEND)
        return false;
    if(opCode >= OpCodes::OP_ADD && opCode < OpCodes::OP_BYTE)
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
        case OP_ISZERO:
            assert(input.size() == 1);
            return input[0] == 0;
        case OP_SUB:
            assert(input.size() == 2);
            return input[0] - input[1];
        case OP_AND:
            assert(input.size() == 2);
            return input[0] & input[1];
        case OP_ADD:
            assert(input.size() == 2);
            return input[0] + input[1];
        case OP_GT:
            assert(input.size() == 2);
            return input[0] > input[1];
        case OP_LT:
            assert(input.size() == 2);
            return input[0] > input[1];
        case OP_XOR:
            assert(input.size() == 2);
            return input[0] ^ input[1];
        case OP_MUL:
            assert(input.size() == 2);
            return input[0] * input[1];
        case OP_MOD:
            assert(input.size() == 2);
            return input[0] % input[1];
        case OP_EQ:
            assert(input.size() == 2);
            return input[0] == input[1];
        default:
            assert(false && "Please add the logic!");
    }
    return 0;
}

bool OpCodes::OpCode::isStackManipulatorOnly() const {
    if(swapNum() != -1)
            return true;
    if(dupNum() != -1)
            return true;
    if(pushNum() != -1)
            return true;
    switch(opCode) {
            case OP_POP:
                    return true;
                default:
                    return false;
            }
}

bool OpCodes::OpCode::isStop() const {
    return opCode == OP_STOP ||
           opCode == OP_RETURN ||
           opCode == OP_INVALID ||
           opCode == OP_SUICIDE;
}

bool OpCodes::OpCode::isFallThrough() const {
    if(isStop())
        return false;

    if(opCode == OP_JUMP)
        return false;

    return true;
}

bool OpCodes::OpCode::isUnknown() const {
    return strncmp(name.c_str(), "UNKNOWN", strlen("UNKNOWN") - 1) == 0;
}
