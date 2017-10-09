#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>
#include "CFStackEntry.h"
#include "Program.h"

class CFInstruction;
class Program;

struct CFNode {
    size_t start = 0,
            end = 0;

    size_t idx = 0;
    bool isJumpDest = false;
    std::__cxx11::string label = "";

    bool IsReachable() const;
    std::set<std::shared_ptr<CFNode>> next, prev;

    std::vector<std::shared_ptr<CFInstruction>> Instructions(const Program& p) const;
    bool hasUnknownOpCodes(const Program& p) const;
    std::shared_ptr<CFInstruction> lastInstruction(const Program& p) const;

    std::map<std::vector<CFStackEntry>, std::vector<std::vector<size_t>> > possibleEntryStackStates;
    std::map<std::vector<CFStackEntry>, std::vector<std::vector<size_t>> > possibleExitStackStates;
};