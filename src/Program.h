#pragma once
#include <stdlib.h>
#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>
#include <ostream>
#include <fstream>

#include "CFStackEntry.h"
#include "CFNode.h"
#include "CFInstruction.h"

struct Program;

typedef std::vector<CFStackEntry> CFStack;
typedef std::vector<size_t> executionPath;
class CFNode;
class CFInstruction;

struct KnownEntryPoint {
    struct Argument {
        std::string type, name;
    };
    std::vector<Argument> arguments;
    std::string name;
    int64_t hash;
};

const KnownEntryPoint* GetKnownEntryPoint(int64_t hash);

struct AnalysisIssue {
    size_t offset;
    std::string message;

    AnalysisIssue(size_t offset, const std::string &message);

    friend std::ostream &operator<<(std::ostream &os, const AnalysisIssue &issue);
};

struct CFSymbolInfo {
    size_t idx;
    size_t createdAt;
    std::set<size_t> usedAt;

    std::string ToString(const Program& p) const;
};

class Program {
    std::map<size_t, std::shared_ptr<CFNode> > nodes;
    std::vector<uint8_t> byteCode;
    std::map<size_t, size_t> jumpdests;
    std::map<size_t, std::shared_ptr<CFInstruction>> instructions;
    std::map<size_t, CFSymbolInfo> symbols;
    std::vector<AnalysisIssue> issues;
    void fillInstructions();

    void initGraph();
public:

    bool IsValid() const;
    void AddIssue(size_t offset, const std::string& msg);
    const std::vector<AnalysisIssue>& Issues() const { return issues; }
    const std::map<size_t, std::shared_ptr<CFInstruction>> &Instructions() const { return instructions; }
    const std::vector<uint8_t>& ByteCode() const { return byteCode; }
    const std::map<size_t, std::shared_ptr<CFNode> >& Nodes() const { return nodes; };

    const std::map<size_t, CFSymbolInfo>& Symbols() const { return symbols; };
    std::shared_ptr<CFNode> GetNodeExactlyAt(size_t offset) const;
    std::shared_ptr<CFNode> GetNode(size_t offset) const;
    std::shared_ptr<CFNode> GetNode(const CFInstruction& instruction) const;
    std::shared_ptr<CFInstruction> GetInstructionByOffset(size_t offset) const {
        auto it = instructions.find(offset);
        if(it != instructions.end()) {
            return it->second;
        }
        return nullptr;
    }

    Program(const std::vector<uint8_t> &byteCode);
    ~Program();

    void print(bool showStackOps, bool showUnreachable);

    void startGraph();

    bool solveStack();

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

class PsuedoStackReport : public ProgramReport {
public:
    PsuedoStackReport(const Program &program);

    std::ostream &Stream(std::ostream &os) const override;
};