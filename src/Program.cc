//
// Created by justin on 10/7/17.
//

#include <set>
#include <iostream>
#include "Program.h"

static void printOpCode(const uint8_t* data, size_t pos, const OpCodes::OpCode& opCode) {
    printf("\t%4lu (0x%04lx): %s", pos, pos, opCode.name.c_str());
    for(size_t i = 0;i < opCode.length;i++) {
        printf(" %02x", data[i]);
    }
    printf("\n");
}

static std::string toString(const std::vector<uint8_t>& data) {
    std::stringstream ss;

    bool isFirst = true;
    for(auto& d : data) {
        if(!isFirst)
            ss << " ";
        ss.width(2);
        ss.fill('0');
        ss << std::hex << (uint32_t)d;
        isFirst = false;
    }
    return ss.str();
}

static std::vector<uint8_t> getVecFromInt64(int64_t _v) {
    uint64_t v = static_cast<uint64_t>(_v);
    std::vector<uint8_t> rtn;
    if(_v < 0)
        for(size_t i = 0;i < 24;i++)
            rtn.push_back(0xff);

    while(v || rtn.empty()) {
        rtn.push_back(v & 0xff);
        v = v >> 8;
    }
    std::reverse(rtn.begin(), rtn.end());
    return rtn;
}

static bool getInt64FromVec(const std::vector<uint8_t>& data, int64_t *rtn) {
    if(data.size() > 8)
        return false;

    int64_t v = 0;
    for(auto& d : data) {
        v = v << 8;
        v += d;
    }
    if(rtn)
        *rtn = v;

    return true;
}

bool CFStackEntry::getConstantInt(int64_t *v) {
    if(!isConstant)
        return false;

    return getInt64FromVec(constantValue, v);
}

std::ostream &operator<<(std::ostream &os, const CFStackEntry &entry) {
    if(entry.isConstant) {
        os << "{" << toString(entry.constantValue) << "}";
    } else {

        if(!entry.label.empty()) {
            os << "<" << entry.label << ".";
        } else {
            os << "<#";
        }
        os << entry.idx << ">";
    }
    return os;
}

bool CFStackEntry::operator==(const CFStackEntry &rhs) const {
    return idx == rhs.idx &&
           label == rhs.label &&
           isConstant == rhs.isConstant &&
           constantValue == rhs.constantValue;
}

bool CFStackEntry::operator!=(const CFStackEntry &rhs) const {
    return !(rhs == *this);
}

bool CFStackEntry::operator<(const CFStackEntry &rhs) const {
    if (idx < rhs.idx)
        return true;
    if (rhs.idx < idx)
        return false;
    if (label < rhs.label)
        return true;
    if (rhs.label < label)
        return false;
    if (isConstant < rhs.isConstant)
        return true;
    if (rhs.isConstant < isConstant)
        return false;
    return constantValue < rhs.constantValue;
}

bool CFStackEntry::operator>(const CFStackEntry &rhs) const {
    return rhs < *this;
}

bool CFStackEntry::operator<=(const CFStackEntry &rhs) const {
    return !(rhs < *this);
}

bool CFStackEntry::operator>=(const CFStackEntry &rhs) const {
    return !(*this < rhs);
}

CFInstruction::CFInstruction(size_t offset, const OpCodes::OpCode &opCode, const std::vector<uint8_t> &data) : offset(offset),
                                                                                                               opCode(opCode),
                                                                                                               data(data) {}

void CFInstruction::simplify() {
    if(opCode.dupNum() != -1) {
        outputs[0] = operands.back();
        for(auto i = 1;i < outputs.size();i++) {
            outputs[i] = operands[i-1];
        }
    } else if(opCode.swapNum() != -1) {
        for(auto i = 0;i < outputs.size();i++) {
            outputs[i] = operands[i];
        }
        std::swap(outputs[0], outputs[outputs.size()-1]);
    }

    if(opCode.isArithmetic()) {
        bool allInputsConstant = true;
        std::vector<int64_t> inputs;

        for(auto& i : operands) {
            int64_t o = 0;
            allInputsConstant &= i.getConstantInt(&o);
            inputs.push_back(o);
        }

        if(allInputsConstant) {
            outputs[0].isConstant = true;
            outputs[0].constantValue = getVecFromInt64(opCode.Solve(inputs));
        }
    }
}

bool CFInstruction::allOperandsConstant() const {
    for(auto& op : operands) {
        if(!op.isConstant)
            return false;
    }
    return true;
}

std::ostream &operator<<(std::ostream &os, const CFInstruction &instruction) {
    return instruction.Stream(os);
}

std::ostream &CFInstruction::Stream(std::ostream& os) const {
    os << "\t";
    os.width(4); os.fill(' '); os << std::dec << offset;
    os << " (0x";
    os.width(4); os.fill('0'); os << std::hex << offset;
    os << "): ";

    if(operands.size() > 0 || outputs.size() > 0) {
        if(outputs.size()) {
            os << "(";
            for(size_t i = 0;i < outputs.size();i++) {
                os << outputs[i];
                if(i != outputs.size() - 1) {
                    os << ", ";
                }
            }
            os << ") := ";
        }
        os << opCode.name << "(";

        for (size_t i = 0; i < operands.size(); i++) {
            os << operands[i];
            if(i != operands.size() - 1)
                os << ", ";
        }
        os << ")";

        os << "\n";
    } else {
        os << opCode.name << "\n";
    }
    return os;
}

void Program::fillInstructions() {
    std::vector<CFStackEntry> stack;
    size_t globalIdx = 0;
    size_t* jumpIdx = 0;
    OpCodes::iterate(byteCode, [&](const uint8_t* data, size_t pos, const OpCodes::OpCode& opCode){
        instructions[pos] = std::make_shared<CFInstruction>(pos, opCode);

        if(opCode.opCode == OpCodes::OP_JUMPDEST) {
            jumpdests[pos] = 0;
            jumpIdx = &jumpdests[pos];
            stack.clear();
        }

        for(size_t i = 0;i < opCode.length;i++) {
            instructions[pos]->data.push_back( data[i]);
        }

        auto stackBack = stack.end();
        for(size_t i = 0;i <opCode.stackRemoved;i++) {
            if(stack.size() == 0) {
                CFStackEntry entry;
                entry.label = "argument";
                entry.idx = (*jumpIdx)++;
                stack.push_back(entry);
            }

            instructions[pos]->operands.emplace_back(stack.back());
            stack.pop_back();
        }

        for(size_t i = 0;i < opCode.stackAdded;i++) {
            CFStackEntry entry;
            entry.idx = globalIdx++;
            if(opCode.opCode >= OpCodes::PUSH1.opCode &&
               opCode.opCode <= OpCodes::PUSH32.opCode) {
                entry.isConstant = true;
                assert(instructions[pos]->data.size());
                entry.constantValue = instructions[pos]->data;
            }
            instructions[pos]->outputs.emplace_back(entry);
        }

        instructions[pos]->simplify();
        for (auto it = instructions[pos]->outputs.rbegin();
             it != instructions[pos]->outputs.rend();it++) {
            stack.push_back(*it);
        }

    });
}

void Program::initGraph() {
    size_t idx = 1;
    CFNode currNode;

    for(auto& inst : instructions) {
        auto& instruction = *inst.second;
        if(currNode.start == (size_t)-1){
            currNode.start = instruction.offset;
        }
        currNode.end = instruction.offset + 1 + instruction.opCode.length;
        if(instruction.opCode.isBranch() ||
                instruction.opCode.isStop()) {
            nodes[currNode.start] = std::make_shared<CFNode>(currNode);
            currNode = CFNode();
            currNode.start = currNode.end = (size_t)-1;
            currNode.idx = idx++;
        } else if(instruction.opCode == OpCodes::JUMPDEST) {
            if(currNode.end - currNode.start == 0) {
                currNode.isJumpDest = true;
            } else {
                currNode.end--;
                nodes[currNode.start] = std::make_shared<CFNode>(currNode);
                currNode = CFNode();
                currNode.start = currNode.end = instruction.offset;
                currNode.isJumpDest = true;
                currNode.idx = idx++;
            }
        }
    }

    if(currNode.start != -1) {
        nodes[currNode.start] = std::make_shared<CFNode>(currNode);
    }
}

void Program::startGraph() {
    std::set< size_t > seen;
    std::vector< size_t > todo;

    for(auto& n : nodes) {
        todo.push_back(n.first);
    }

    while(!todo.empty()) {
        auto pos = todo.back(); todo.pop_back();

        if(seen.find(pos) != seen.end())
            continue;
        seen.insert(pos);

        auto node = nodes[pos];
        assert(node);
        auto lastInstr = node->lastInstruction(*this);
        assert(lastInstr);

        if( lastInstr->opCode.isFallThrough()) {
            auto& next = nodes[ node->end];
            if(next) {
                node->next.insert(next);
                next->prev.insert(node);
            }
        }

        if( lastInstr->opCode.opCode == OpCodes::OP_JUMPI ||
                lastInstr->opCode.opCode == OpCodes::OP_JUMP) {
            assert(!lastInstr->operands.empty());
            auto& jumpTo = lastInstr->operands.front();
            int64_t nextAddr = 0;
            if(jumpTo.isConstant && getInt64FromVec(jumpTo.constantValue, &nextAddr)) {
                if(auto& next = nodes[ nextAddr ]) {
                    node->next.insert(next);
                    next->prev.insert(node);
                }
            }
        }

    }
}

void Program::solveStack(size_t& globalIdx,
                         std::shared_ptr<CFNode> node,
                         std::shared_ptr<CFNode> pnode) {
    std::map< CFStack, std::vector<executionPath> >& possibleStackStarts =
            node->possibleEntryStackStates;

    if(pnode) {
        for(auto& s : pnode->possibleExitStackStates) {
            auto path = s.second;
            for(auto& p : path) {
                p.push_back(pnode->idx);
                possibleStackStarts[s.first].push_back(p);
            }
        }
    }
    else {
        assert(node->idx == 0);
        CFStack empty;
        possibleStackStarts[empty].emplace_back();
    }

    auto nodeInstructions = node->Instructions(*this);

    for(auto& _possibleStackStart : possibleStackStarts) {
        auto stack = _possibleStackStart.first;

        for(auto& instruction : nodeInstructions) {
            auto& opCode = instruction->opCode;
            auto pos = instruction->offset;

            auto oldOperands = instruction->operands;
            auto oldOutputs = instruction->outputs;

            instruction->operands.clear();
            instruction->outputs.clear();

            auto stackBack = stack.end();
            for (size_t i = 0; i < opCode.stackRemoved; i++) {
                instruction->operands.emplace_back(stack.back());
                stack.pop_back();
            }

            for (size_t i = 0; i < opCode.stackAdded; i++) {
                CFStackEntry entry;
                entry.idx = globalIdx++;
                if (opCode.opCode >= OpCodes::PUSH1.opCode &&
                    opCode.opCode <= OpCodes::PUSH32.opCode) {
                    entry.isConstant = true;
                    assert(instruction->data.size());
                    entry.constantValue = instruction->data;
                }
                instruction->outputs.emplace_back(entry);
            }

            instruction->simplify();
            for (auto it = instruction->outputs.rbegin();
                    it != instruction->outputs.rend();it++) {
                stack.push_back(*it);
            }

            int64_t jumpLoc = 0;
            if(instruction->opCode.isBranch() &&
               !instruction->operands.empty() &&
               instruction->operands.front().isConstant &&
               getInt64FromVec(instruction->operands.front().constantValue, &jumpLoc)) {
                if(nodes[jumpLoc]) {
                    node->next.insert(nodes[jumpLoc]);
                    nodes[jumpLoc]->prev.insert(node);
                }
            }

            instruction->operands = oldOperands;
            instruction->outputs = oldOutputs;
        }

        for(auto& p : _possibleStackStart.second)
            node->possibleExitStackStates[stack].push_back(p);
    }
}

void Program::solveStack() {
    typedef std::pair< std::shared_ptr<CFNode>,
            std::shared_ptr<CFNode> > NodePair;
    std::set< NodePair > seen;
    std::vector< NodePair > todo;
    size_t globalIdx = 0;

    todo.emplace_back(nodes[0], nullptr);
    while(!todo.empty()) {
        auto pos = todo.back();
        todo.pop_back();

        if (seen.find(pos) != seen.end())
            continue;
        seen.insert(pos);

        auto node = pos.first;
        assert(node);

        solveStack(globalIdx, node, pos.second);

        for(auto& n : node->next) {
                todo.emplace_back(n, node);
        }
    }
}

Program::Program(const std::vector<uint8_t> &byteCode) : byteCode(byteCode) {
    fillInstructions();
    initGraph();
    startGraph();
    solveStack();

    findCreatedContracts();
}

void Program::print(bool showStackOps, bool showUnreachable) {
    std::cout << DisassemReport(*this, showStackOps, showUnreachable);
}

std::ostream& Program::streamStackStates(std::ostream& os, const std::map<CFStack, std::vector<executionPath> > &stackStates) const {
    for(auto& ps : stackStates) {
            auto& s = ps.first;
            auto& paths = ps.second;

        os << "For execution paths: ";
            for(auto& path : paths) {
                for(auto& node : path) {
                    os << node << "-";
                }
                os << ", ";
            }
        os << std::endl;
            for(int32_t i = s.size() - 1;i >= 0;i--) {
                os << "\t[";
                os.width(3); os.fill(' ');
                os << (s.size() - i - 1) << "]: ";
                os << s[i] << std::endl;
            }
        }
}

void Program::findCreatedContracts() {
    for(auto& _node : nodes) {
        auto& node = _node.second;
        if(!node) continue;

        auto nodeInstructions = node->Instructions(*this);
        if(!node->IsReachable())
            continue;
        for (auto &instr : nodeInstructions) {
            if (!instr)
                continue;

            if (instr->opCode.opCode == OpCodes::OP_CODECOPY && instr->allOperandsConstant()) {
                int64_t mLoc, mOffset, mSize;
                bool canRead =
                        getInt64FromVec(instr->operands[0].constantValue, &mLoc) &&
                        getInt64FromVec(instr->operands[1].constantValue, &mOffset) &&
                        getInt64FromVec(instr->operands[2].constantValue, &mSize);
                assert(canRead);

                auto pos = instr->offset;
                while (pos < instructions.size()) {
                    auto instr = instructions[pos++];
                    if (instr && instr->opCode.isStop()) {

                        if (instr->opCode.opCode == OpCodes::OP_RETURN && instr->allOperandsConstant()) {
                            int64_t rLoc, rSize;
                            bool canRead =
                                    getInt64FromVec(instr->operands[0].constantValue, &rLoc) &&
                                    getInt64FromVec(instr->operands[1].constantValue, &rSize);
                            assert(canRead);

                            std::vector<uint8_t> newBC;
                            auto offset = rLoc - mLoc;
                            for (int i = mOffset - offset; i < mOffset + rSize - offset; i++) {
                                newBC.push_back(byteCode[i]);
                            }

                            createdContracts.emplace_back(std::make_shared<Program>(newBC));
                        }

                        break;
                    }
                }
            }
        }

    }
}

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
        if(it != p.Instructions().end()) {
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
    return idx == 0 || !prev.empty();
}

std::ostream &operator<<(std::ostream &os, const ProgramReport &report) {
    return report.Stream(os);
}

ProgramReport::ProgramReport(const Program &program) : program(program) {}

DisassemReport::DisassemReport(const Program &program, bool shouldPrintStackOps, bool shouldShowUnreachable)
        : ProgramReport(program), shouldPrintStackOps(shouldPrintStackOps),
          shouldShowUnreachable(shouldShowUnreachable) {}

std::ostream &DisassemReport::Stream(std::ostream &os) const {
    os << "entry:" << std::endl;
    for(auto& pr : program.Nodes()) {
        auto& node = pr.second;
        if(!node)
            continue;

        if(!node->IsReachable() && !shouldShowUnreachable)
            continue;

        if(node->isJumpDest) {
            os << "loc_" << node->idx << ": " << std::endl;
        } else {
            os << "/* Block " << std::dec << node->idx << "*/" << std::endl;
        }
        if(!node->IsReachable() && node->hasUnknownOpCodes(program)) {
            os << "/* Possible data section: */" << std::endl;
            for(auto i = node->start;i < node->end;i++ ) {
                if((i - node->start) % 16 == 0 && i != node->start)
                    os << std::endl;
                os.fill('0');
                os.width(2);
                os << std::hex << (uint32_t)program.ByteCode()[i] << " ";
            }
            os << std::endl;
            continue;
        }

        if(!node->IsReachable()) {
            os << "/* Unreachable*/" << std::endl;
        } else {
            os << "/* Reachable from ";
            for(auto& n : node->prev) {
                os << n->idx << " ";
            }
            os << "*/" << std::endl;
        }

        os << "Entry states:" << std::endl;
        program.streamStackStates(os, node->possibleEntryStackStates);
        for(size_t i = node->start;i < node->end;i++) {
            //if(instructions[i] && !instructions[i]->opCode.isStackManipulatorOnly())
            if(auto instr = program.GetInstructionByOffset(i))
                if(shouldPrintStackOps || !instr->opCode.isStackManipulatorOnly())
                    os << *instr;
        }

        os << "Can go to: ";
        for(auto& n : node->next) {
            os << n->idx << ", ";
        }
        os << std::endl;

        os << "Exit states:" << std::endl;
        program.streamStackStates(os, node->possibleExitStackStates);
        os << std::endl;
    }

    if(!program.createdContracts.empty()) {
        for(auto& cc : program.createdContracts) {
            os << "Can Create contract:" << std::endl;
            cc->print(shouldPrintStackOps, shouldShowUnreachable);
        }
    }

    return os;
}
