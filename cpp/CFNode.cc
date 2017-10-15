

#include "Program.h"
#include <set>
#include "CFNode.h"
#include "CFInstruction.h"
std::shared_ptr<CFInstruction> CFNode::lastInstruction(const Program& p) const {
    for(size_t last = end - 1; last >= start; last--) {
        auto it = p.Instructions().find(last);
        if(it != p.Instructions().end()) {
            return it->second;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<CFInstruction>> CFNode::Instructions(const Program &p) const {
    std::vector<std::shared_ptr<CFInstruction>> rtn;
    for(size_t i = start; i < end; i++) {
        auto it = p.Instructions().find(i);
        if(it != p.Instructions().end() && it->second) {
            rtn.push_back(it->second);
        }
    }
    return rtn;
}

bool CFNode::hasUnknownOpCodes(const Program &p) const {
    auto instrs = Instructions(p);
    for(auto& instr : instrs) {
        if(instr->opCode.isUnknown())
            return true;
    }
    return false;
}

bool CFNode::IsReachable() const {
    if(isReachable || idx == 0)
        return true;

    if(isReachableStale) {
        isReachableStale = false;

        if(prev.empty()) {
            return false;
        }
        for(auto& p : PrevNodes()) {
            if(p->IsReachable()) {
                return isReachable = true;
            }
        }
    }
    return false;
}

void CFNode::AddNext(const std::shared_ptr<CFNode> &next) {
    this->next.insert(next);
    next->prev.insert(this->shared_from_this());
    next->isReachableStale = true;
}

bool CFNode::HasPossibleEntryStackStates() const {
    for(auto& m : possibleEntryStackStates) {
        if(!m.first.empty())
            return true;
    }
    return false;
}

bool CFNode::HasPossibleExitStackStates() const {
    for(auto& m : possibleExitStackStates) {
        if(!m.first.empty())
            return true;
    }
    return false;
}

static const std::set<std::shared_ptr<CFNode>> weakSetToShared(const std::set<std::weak_ptr<CFNode>>& set) {
    std::set<std::shared_ptr<CFNode>> rtn;
    for(auto& s : set)
        if(auto i = s.lock())
            rtn.insert(i);
    return rtn;
}

const std::set<std::shared_ptr<CFNode>>& CFNode::NextNodes() const {
    return next;
}

const std::set<std::shared_ptr<CFNode>>& CFNode::PrevNodes() const {
    return prev;
}

