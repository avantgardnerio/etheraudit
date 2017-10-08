#include <fstream>
#include <vector>
#include <assert.h>
#include <iostream>
#include <memory>
#include <map>
#include <sstream>
#include <algorithm>

#include "OpCodes.h"

static void printOpCode(const uint8_t* data, size_t pos, const OpCodes::OpCode& opCode) {
    printf("\t%4lu (0x%04lx): %s", pos, pos, opCode.name.c_str());
    for(size_t i = 0;i < opCode.length;i++) {
        printf(" %02x", data[i]);
    }
    printf("\n");
}


struct CFNode {
    size_t start = 0,
            end = 0;

    size_t idx = 0;
    bool isJumpDest = false;
    std::string label = "";

    std::vector<std::shared_ptr<CFNode>> next;
};

std::string toString(const std::vector<uint8_t>& data) {
    std::stringstream ss;
    ss.width(2);
    ss.fill('0');
    bool isFirst = true;
    for(auto& d : data) {
        if(!isFirst)
            ss << " ";
        ss << std::hex << (uint32_t)d;
        isFirst = false;
    }
    return ss.str();
}
std::vector<uint8_t> getVecFromInt64(int64_t _v) {
    uint64_t v = static_cast<uint64_t>(_v);
    std::vector<uint8_t> rtn;
    if(_v < 0)
        for(size_t i = 0;i < 24;i++)
            rtn.push_back(0xff);

    while(v) {
        rtn.push_back(v & 0xff);
        v = v >> 8;
    }
    std::reverse(rtn.begin(), rtn.end());
    return rtn;
}
bool getInt64FromVec(const std::vector<uint8_t>& data, int64_t *rtn) {
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

struct CFStackEntry {
    size_t globalIdx;
    bool isConstant = false;
    std::vector<uint8_t> constantValue;

    bool getConstantInt(int64_t* v) {
        if(!isConstant)
            return false;

        return getInt64FromVec(constantValue, v);
    }

    friend std::ostream &operator<<(std::ostream &os, const CFStackEntry &entry) {
        if(entry.isConstant) {
            os << toString(entry.constantValue);
        } else {
            os << "<#" << entry.globalIdx << ">";
        }
        return os;
    }
};

struct CFInstruction {
    size_t offset;
    const OpCodes::OpCode& opCode;
    std::vector<uint8_t> data;

    CFInstruction(size_t offset, const OpCodes::OpCode &opCode, const std::vector<uint8_t> &data = {}) : offset(offset),
                                                                                                    opCode(opCode),
                                                                                                    data(data) {}

    std::vector<CFStackEntry> operands;
    std::vector<CFStackEntry> outputs;

    void simplify() {
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

    void print() {
        printOpCode(data.data(), offset, opCode);
        std::stringstream ss;
        if(operands.size() > 0 || outputs.size() > 0) {
            ss << "\t\t " << opCode.name << "(";

            for (size_t i = 0; i < operands.size(); i++) {
                ss << operands[i];
                if(i != operands.size() - 1)
                    ss << ", ";
            }
            ss << ")";
            if(outputs.size()) {
                ss << " = (";
                for(size_t i = 0;i < outputs.size();i++) {
                    ss << outputs[i];
                    if(i != outputs.size() - 1) {
                        ss << ", ";
                    }
                }
                ss << ")";
            }
            ss << "\n";
            printf("%s", ss.str().c_str());
        }
    }
};

class Program {
    std::vector<std::shared_ptr<CFNode> > nodes;
    std::vector<uint8_t> byteCode;
    std::map<size_t, std::shared_ptr<CFInstruction>> instructions;

    void fillInstructions() {
        std::vector<CFStackEntry> stack;
        size_t globalIdx = 0;
        OpCodes::iterate(byteCode, [&](const uint8_t* data, size_t pos, const OpCodes::OpCode& opCode){
            instructions[pos] = std::make_shared<CFInstruction>(pos, opCode);
            for(size_t i = 0;i < opCode.length;i++) {
                instructions[pos]->data.push_back( data[i]);
            }

            auto stackBack = stack.end();
            for(size_t i = 0;i <opCode.stackRemoved;i++) {
                instructions[pos]->operands.emplace_back(stack.back());
                stack.pop_back();
            }

            for(size_t i = 0;i < opCode.stackAdded;i++) {
                CFStackEntry entry;
                entry.globalIdx = globalIdx++;
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

    void fillGraph() {
        size_t idx = 0;
        CFNode currNode;

        for(auto& inst : instructions) {
            auto& instruction = *inst.second;
            if(currNode.start == (size_t)-1){
                currNode.start = instruction.offset;
            }
            currNode.end = instruction.offset + 1;
            if(instruction.opCode.isBranch()) {
                nodes.emplace_back(std::make_shared<CFNode>(currNode));
                currNode = CFNode();
                currNode.start = currNode.end = (size_t)-1;
                currNode.idx = idx++;
            } else if(instruction.opCode == OpCodes::JUMPDEST) {
                if(currNode.end - currNode.start == 0) {
                    currNode.isJumpDest = true;
                } else {
                    currNode.end--;
                    nodes.emplace_back(std::make_shared<CFNode>(currNode));
                    currNode = CFNode();
                    currNode.start = currNode.end = instruction.offset;
                    currNode.idx = idx++;
                }
            }
        }
    }
public:
    const std::map<size_t, std::shared_ptr<CFInstruction>> &Instructions() const {
        return instructions;
    }

    Program(std::vector<uint8_t> byteCode) : byteCode(byteCode) {
        fillInstructions();
        fillGraph();
    }

    void print() {
        printf("entry:\n");
        for(auto& node : nodes) {
            if(node->isJumpDest) {
                printf("loc_%ld:\n", node->idx);
            } else {
                printf("/*%ld:/*\n", node->idx);
            }
            for(size_t i = node->start;i < node->end;i++) {
                if(instructions[i] && !instructions[i]->opCode.isStackManipulatorOnly())
                    instructions[i]->print();
            }
        }
    }
};

void printByteCode(const std::vector<uint8_t>& bc) {
    OpCodes::iterate(bc, &printOpCode);
}

std::vector<uint8_t> parseByteCodeString(const std::string& str) {
  const char* buff = str.c_str();
  if(buff[1] == 'x')
    buff += 2; // Eat '0x'

  char scratch[3] = {};
  
  std::vector<uint8_t> rtn; 
  while(*buff) {
    scratch[0] = buff[0];
    scratch[1] = buff[1];
    
    rtn.push_back( strtol(scratch, 0, 16) );
    buff += 2;
  }

  return rtn;
}

std::string readFile(std::ifstream& fs) {
  std::string rtn, line;
  while( std::getline(fs, line) )
    rtn += line;
  return rtn; 
}

int main(int argc, const char**argv) {
  std::ifstream f(argv[1]);

  auto bc = parseByteCodeString(readFile(f));
  //printByteCode(bc);

  Program p(bc);
  p.print();
  auto pt = p.Instructions().find(70)->second.get();
  return 0;
}
