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
    }
};

class Program {
    std::vector<std::shared_ptr<CFNode> > nodes;
    std::vector<uint8_t> byteCode;
    std::map<size_t, std::shared_ptr<CFInstruction>> instructions;

    void fillInstructions() {
        OpCodes::iterate(byteCode, [&](const uint8_t* data, size_t pos, const OpCodes::OpCode& opCode){
            instructions[pos] = std::make_shared<CFInstruction>(pos, opCode);
            for(size_t i = 0;i < opCode.length;i++) {
                instructions[pos]->data.push_back( data[i]);
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
            currNode.end = instruction.offset;
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
    Program(std::vector<uint8_t> byteCode) : byteCode(byteCode) {
        fillInstructions();
        fillGraph();
    }

    void print() {
        printf("entry:\n");
        for(auto& node : nodes) {
            if(node->isJumpDest) {
                printf("loc_%ld:\n", node->idx);
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
  return 0;
}
