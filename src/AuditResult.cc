
#include "CFInstruction.h"
#include "AuditResult.h"

AuditResults AuditForUncheckedExternalCall(const Program &program) {
    AuditResults rtn;
    return rtn;
}


AuditResults AuditForOriginRead(const Program &program) {
    static AuditClass OriginRead(1, "Contract is checking tx.origin instead of tx.sender");

    AuditResults rtn;

    // Not an issue at all in ctor contracts
    if(!program.createdContracts.empty())
        return rtn;

    for(auto i : program.Instructions()) {
        if(auto instr = i.second) {
           if(instr->opCode.opCode == OpCodes::OP_ORIGIN &&
                   program.GetNode(*instr)->IsReachable()) {
               rtn.emplace_back(i.first, OriginRead);
           }
        }
    }

    return rtn;
}


AuditResults AuditForEverything(const Program &program) {
    AuditResults rtn = AuditForOriginRead(program);
    return rtn;
}

AuditResult::AuditResult(size_t offset, const AuditClass &type) : offset(offset), type(type) {}

AuditClass::AuditClass(size_t severity, const std::string &message) : severity(severity), message(message) {}
