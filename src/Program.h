#pragma once
#include <stdlib.h>
#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>
#include <ostream>

#include "OpCodes.h"

struct CFInstruction;
struct Program;

struct CFStackEntry {
    size_t idx;
    std::string label = "";
    bool isConstant = false;
    std::vector<uint8_t> constantValue;

    bool getConstantInt(int64_t* v);

    friend std::ostream &operator<<(std::ostream &os, const CFStackEntry &entry);

    bool operator==(const CFStackEntry &rhs) const;

    bool operator!=(const CFStackEntry &rhs) const;

    bool operator<(const CFStackEntry &rhs) const;

    bool operator>(const CFStackEntry &rhs) const;

    bool operator<=(const CFStackEntry &rhs) const;

    bool operator>=(const CFStackEntry &rhs) const;
};
typedef std::vector<CFStackEntry> CFStack;
typedef std::vector<size_t> executionPath;

struct CFNode {
    size_t start = 0,
            end = 0;

    size_t idx = 0;
    bool isJumpDest = false;
    std::string label = "";

    bool IsReachable() const;
    std::set<std::shared_ptr<CFNode>> next, prev;

    std::vector<std::shared_ptr<CFInstruction>> Instructions(const Program& p) const;
    bool hasUnknownOpCodes(const Program& p) const;
    std::shared_ptr<CFInstruction> lastInstruction(const Program& p) const;

    std::map< CFStack, std::vector<executionPath> > possibleEntryStackStates;
    std::map< CFStack, std::vector<executionPath> > possibleExitStackStates;
};

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

class Program {
    std::map<size_t, std::shared_ptr<CFNode> > nodes;
    std::vector<uint8_t> byteCode;
    std::map<size_t, size_t> jumpdests;
    std::map<size_t, std::shared_ptr<CFInstruction>> instructions;

    void fillInstructions();

    void initGraph();
public:
    const std::map<size_t, std::shared_ptr<CFInstruction>> &Instructions() const { return instructions; }
    const std::vector<uint8_t>& ByteCode() const { return byteCode; }
    const std::map<size_t, std::shared_ptr<CFNode> >& Nodes() const { return nodes; };

    std::shared_ptr<CFInstruction> GetInstructionByOffset(size_t offset) const {
        auto it = instructions.find(offset);
        if(it != instructions.end()) {
            return it->second;
        }
        return nullptr;
    }

    Program(const std::vector<uint8_t> &byteCode);

    void print(bool showStackOps, bool showUnreachable);

    void startGraph();

    void solveStack();

    void solveStack(size_t& globalIdx,
                    std::shared_ptr<CFNode> node,
                    std::shared_ptr<CFNode> pnode);

    std::ostream& streamStackStates(std::ostream& os, const std::map<CFStack, std::vector<executionPath> > &stackStates) const;

    std::vector<std::shared_ptr<Program>> createdContracts;

    void findCreatedContracts();

    friend class ProgramReport;
};

class ProgramReport {
protected:
    const Program& program;
public:
    ProgramReport(const Program &program);

    virtual std::ostream& Stream(std::ostream& os) const = 0;
    friend std::ostream &operator<<(std::ostream &os, const ProgramReport &report);
};

class DisassemReport : public ProgramReport {
    bool shouldPrintStackOps, shouldShowUnreachable;
public:
    DisassemReport(const Program &program, bool shouldPrintStackOps, bool shouldShowUnreachable);

    std::ostream &Stream(std::ostream &os) const override;
};