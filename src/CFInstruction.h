

#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>
#include "OpCodes.h"
#include "CFExpression.h"
#include "CFNode.h"

struct CFInstruction;

struct CFInstruction {
    size_t offset;
    const OpCodes::OpCode& opCode;
    std::vector<uint8_t> data;

    CFInstruction(const Program &program, size_t offset, const OpCodes::OpCode &opCode, const std::vector<uint8_t> &data = {});

    std::vector<CFExpression> operands;
    std::vector<CFExpression> outputs;

    bool allOutputsSingleUse() const;
    bool allOperandsConstant() const;

    void simplify();

    std::ostream &Stream(std::ostream &os, bool showAllOps) const;

    const Program& program;

    friend std::ostream &operator<<(std::ostream &os, const CFInstruction &instruction);
};