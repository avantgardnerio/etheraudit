//
// Created by justin on 10/7/17.
//

#include <set>
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

void CFInstruction::print() {
    //printOpCode(data.data(), offset, opCode);
    std::stringstream ss;
    if(operands.size() > 0 || outputs.size() > 0) {
        if(outputs.size()) {
            ss << "(";
            for(size_t i = 0;i < outputs.size();i++) {
                ss << outputs[i];
                if(i != outputs.size() - 1) {
                    ss << ", ";
                }
            }
            ss << ") := ";
        }
        ss << opCode.name << "(";

        for (size_t i = 0; i < operands.size(); i++) {
            ss << operands[i];
            if(i != operands.size() - 1)
                ss << ", ";
        }
        ss << ")";

        ss << "\n";
    } else {
        ss << opCode.name << "\n";
    }
    printf("\t%4lu (0x%04lx): %s", offset, offset, ss.str().c_str());
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
        for(auto& o : instructions[pos]->outputs) {
            stack.push_back(o);
        }

    });
}

void Program::initGraph() {
    size_t idx = 1;
    CFNode currNode;
    currNode.isReachable = true;

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

    todo.push_back(0);
    while(!todo.empty()) {
        auto pos = todo.back(); todo.pop_back();

        if(seen.find(pos) != seen.end())
            continue;
        seen.insert(pos);

        auto node = nodes[pos];
        assert(node);
        auto lastInstr = node->lastInstruction(*this);
        assert(lastInstr);
        node->isReachable = true;

        if( lastInstr->opCode.isFallThrough()) {
            auto& next = nodes[ node->end];
            assert(next);
            node->next.push_back(next);
            next->prev.push_back(node);
            todo.push_back(node->end);
        }

        if( lastInstr->opCode.opCode == OpCodes::OP_JUMPI ||
                lastInstr->opCode.opCode == OpCodes::OP_JUMP) {
            assert(!lastInstr->operands.empty());
            auto& jumpTo = lastInstr->operands.front();
            int64_t nextAddr = 0;
            if(jumpTo.isConstant && getInt64FromVec(jumpTo.constantValue, &nextAddr)) {
                auto& next = nodes[ nextAddr ];
                assert(next);
                node->next.push_back(next);
                next->prev.push_back(node);
                todo.push_back(nextAddr);
            }
        }

    }
}

Program::Program(std::vector<uint8_t> byteCode) : byteCode(byteCode) {
    fillInstructions();
    initGraph();
    startGraph();
}

void Program::print() {
    printf("entry:\n");
    for(auto& pr : nodes) {
        auto& node = pr.second;
        if(node->isJumpDest) {
            printf("loc_%ld:\n", node->idx);
        } else {
            printf("/*%ld:/*\n", node->idx);
        }
        if(!node->isReachable && node->hasUnknownOpCodes(*this)) {
            printf("/* Possible data section: */\n");
            for(auto i = node->start;i < node->end;i++ ) {
                if((i - node->start) % 16 == 0 && i != node->start)
                    printf("\n");
                printf("%02x ", byteCode[i]);
            }
            continue;
        }

        if(!node->isReachable) {
            printf("/*Unreachable*/\n");
        } else {
            std::stringstream ss;
            for(auto& n : node->prev) {
                ss << n->idx << " ";
            }
            printf("/*Reachable from %s*/\n", ss.str().c_str());
        }
        for(size_t i = node->start;i < node->end;i++) {
            if(instructions[i] && !instructions[i]->opCode.isStackManipulatorOnly())
                //if(instructions[i])
                instructions[i]->print();
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
