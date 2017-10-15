cmake_minimum_required(VERSION 3.7)

project(etheraudit)

add_executable(etherdis etherdis.cc opcodes_xx.h
        OpCodes.cc OpCodes.h
        Program.cc Program.h
        CFExpression.cc CFExpression.h
        CFNode.cc CFNode.h
        CFInstruction.cc CFInstruction.h
        Utils.cc Utils.h AuditResult.cc AuditResult.h)
