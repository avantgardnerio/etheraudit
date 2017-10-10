

#include "Program.h"
#include <iostream>
#include <set>
#include "CFInstruction.h"
#include "Utils.h"

CFInstruction::CFInstruction(const Program &program, size_t offset, const OpCodes::OpCode &opCode,
                             const std::vector<uint8_t> &data) : program(program), offset(offset),
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
        std::swap(outputs[0], outputs[outputs.size() - 1]);
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
            try {
                outputs[0].constantValue = getVecFromInt64(opCode.Solve(inputs));
                outputs[0].isConstant = true;
            } catch(std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
    }
}


bool CFInstruction::allOutputsSingleUse() const {
    if(outputs.empty())
        return false;
    
    for(auto& op : outputs) {
        if(op.isSymbol()) {
            auto it = program.Symbols().find(op.idx);
            assert(it != program.Symbols().end());
            if(it != program.Symbols().end() && it->second.usedAt.size() > 1) {
                return false;
            }
        }
    }
    return true;
}

bool CFInstruction::allOperandsConstant() const {
    for(auto& op : operands) {
        if(!op.isConstant)
            return false;
    }
    return true;
}

std::ostream &operator<<(std::ostream &os, const CFInstruction &instruction) {
    return instruction.Stream(os, true);
}

std::ostream &CFInstruction::Stream(std::ostream &os, bool showAllOps) const {
    if(allOutputsSingleUse() && !showAllOps)
        return os;

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

            auto it = program.Symbols().find(operands[i].idx);
            if(!showAllOps && it != program.Symbols().end() && it->second.usedAt.size() == 1) {
                os << it->second.ToString(program);
            } else {
                os << operands[i];
            }
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
