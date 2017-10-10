#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <set>
#include <unordered_set>
#include "CFStackEntry.h"
#include "Program.h"

class CFInstruction;
class Program;

class CFNode : public std::enable_shared_from_this<CFNode> {
    mutable bool isReachable = false, isReachableStale = true;
    std::set<std::shared_ptr<CFNode>> next, prev;
public:
    size_t start = 0,
            end = 0;

    size_t idx = 0;
    bool isJumpDest = false;
    std::string label = "";

    bool IsReachable() const;
    void ClearNextAndPrev() {
        next.clear();
        prev.clear();
    }
    const std::set<std::shared_ptr<CFNode>>& NextNodes() const;
    const std::set<std::shared_ptr<CFNode>>& PrevNodes() const;
    void AddNext(const std::shared_ptr<CFNode>& next);
    std::vector<std::shared_ptr<CFInstruction>> Instructions(const Program& p) const;
    bool hasUnknownOpCodes(const Program& p) const;
    std::shared_ptr<CFInstruction> lastInstruction(const Program& p) const;

    bool HasPossibleEntryStackStates() const;
    std::map<std::vector<CFStackEntry>, std::vector<std::vector<size_t>> > possibleEntryStackStates;

    bool HasPossibleExitStackStates() const;
    std::map<std::vector<CFStackEntry>, std::vector<std::vector<size_t>> > possibleExitStackStates;
};