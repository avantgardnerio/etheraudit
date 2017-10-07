#include <fstream>
#include <vector>
#include <assert.h>
#include <iostream>
#include <memory>
#include <map>

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

struct CFStackEntry {
    std::vector<size_t> fromOffset;
    bool isConstant = false;
    std::vector<uint8_t> constantValue;
};

struct CFInstruction {
    size_t offset;
    const OpCodes::OpCode& opCode;
    std::vector<uint8_t> data;

    CFInstruction(size_t offset, const OpCodes::OpCode &opCode, const std::vector<uint8_t> &data = {}) : offset(offset),
                                                                                                    opCode(opCode),
                                                                                                    data(data) {}

    std::vector<CFStackEntry> operands;
    void print() {
        printOpCode(data.data(), offset, opCode);

        if(operands.size()> 0) {
            printf("\t\t");
            for (size_t i = 0; i < operands.size(); i++) {
                if(operands[i].isConstant) {
                    for(auto& b : operands[i].constantValue) {
                        printf(" %02x", b);
                    }
                } else {
                    printf(" <#%lu>", operands[i].fromOffset[0]);
                }
                printf(",");
            }
            printf("\n");
        }
    }
};

class Program {
    std::vector<std::shared_ptr<CFNode> > nodes;
    std::vector<uint8_t> byteCode;
    std::map<size_t, std::shared_ptr<CFInstruction>> instructions;

    void fillInstructions() {
        std::vector<CFStackEntry> stack;

        OpCodes::iterate(byteCode, [&](const uint8_t* data, size_t pos, const OpCodes::OpCode& opCode){
            instructions[pos] = std::make_shared<CFInstruction>(pos, opCode);
            for(size_t i = 0;i < opCode.length;i++) {
                instructions[pos]->data.push_back( data[i]);
            }

            for(size_t i = 0;i <opCode.stackRemoved;i++) {
                instructions[pos]->operands.emplace_back(stack.back());
                stack.pop_back();
            }

            for(size_t i = 0;i < opCode.stackAdded;i++) {
                CFStackEntry entry;
                entry.fromOffset.push_back(pos);
                if(opCode.opCode >= OpCodes::PUSH1.opCode &&
                        opCode.opCode <= OpCodes::PUSH32.opCode) {
                    entry.isConstant = true;
                    assert(instructions[pos]->data.size());
                    entry.constantValue = instructions[pos]->data;
                }
                stack.push_back(entry);
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
                if(instructions[i])
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
