#pragma once
#include <stdlib.h>
#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include "OpCodes.h"

struct CFNode {
    size_t start = 0,
            end = 0;

    size_t idx = 0;
    bool isJumpDest = false;
    std::string label = "";

    std::vector<std::shared_ptr<CFNode>> next;
};

struct CFStackEntry {
    size_t idx;
    std::string label = "";
    bool isConstant = false;
    std::vector<uint8_t> constantValue;

    bool getConstantInt(int64_t* v);

    friend std::ostream &operator<<(std::ostream &os, const CFStackEntry &entry);
};

struct CFInstruction {
    size_t offset;
    const OpCodes::OpCode& opCode;
    std::vector<uint8_t> data;

    CFInstruction(size_t offset, const OpCodes::OpCode &opCode, const std::vector<uint8_t> &data = {});

    std::vector<CFStackEntry> operands;
    std::vector<CFStackEntry> outputs;

    void simplify();

    void print();
};

class Program {
    std::vector<std::shared_ptr<CFNode> > nodes;
    std::vector<uint8_t> byteCode;
    std::map<size_t, size_t> jumpdests;
    std::map<size_t, std::shared_ptr<CFInstruction>> instructions;

    void fillInstructions();

    void fillGraph();
public:
    const std::map<size_t, std::shared_ptr<CFInstruction>> &Instructions() const {
        return instructions;
    }

    Program(std::vector<uint8_t> byteCode);

    void print();
};
