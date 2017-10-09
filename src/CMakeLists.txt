cmake_minimum_required(VERSION 3.7)

project(etheraudit)

add_executable(etherdis etherdis.cc opcodes_xx.h
        OpCodes.cc OpCodes.h
        Program.cc Program.h)
