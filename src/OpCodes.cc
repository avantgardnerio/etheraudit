#include "OpCodes.h"
#include <iostream>
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
