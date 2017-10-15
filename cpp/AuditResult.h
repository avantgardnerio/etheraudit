#pragma once

#include <stdlib.h>
#include <string>
#include <memory>
#include <vector>
#include "Program.h"

class AuditClass {
    size_t severity;
    std::string message;

public:
    size_t Severity() const { return severity; }
    const std::string& Message() const { return message; }
    AuditClass(size_t severity, const std::string &message);
};

class AuditResult {
    size_t offset;
    const AuditClass& type;

public:
    size_t Offset() const { return offset; }
    const AuditClass& Type() const { return type; }
    AuditResult(size_t offset, const AuditClass &type);
};

typedef std::vector<AuditResult> AuditResults;


AuditResults AuditForUncheckedExternalCall(const Program& program);
AuditResults AuditForOriginRead(const Program& program);
AuditResults AuditForMsgSenderSave(const Program& program);
AuditResults AuditForExternalCallBeforeStateChange(const Program& program);

AuditResults AuditForEverything(const Program& program);