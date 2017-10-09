

#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>
#include "OpCodes.h"
#include "CFStackEntry.h"
#include "CFNode.h"

struct CFInstruction;

struct CFInstruction {
    size_t offset;
    const OpCodes::OpCode& opCode;
    std::vector<uint8_t> data;

    CFInstruction(size_t offset, const OpCodes::OpCode &opCode, const std::vector<uint8_t> &data = {});

    std::vector<CFStackEntry> operands;
    std::vector<CFStackEntry> outputs;

    bool allOperandsConstant() const;

    void simplify();

    std::ostream& Stream(std::ostream& os) const;

    friend std::ostream &operator<<(std::ostream &os, const CFInstruction &instruction);
};