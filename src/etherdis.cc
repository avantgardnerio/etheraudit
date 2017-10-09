#include <fstream>
#include <vector>
#include <assert.h>
#include <iostream>
#include <memory>
#include <map>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>

#include "OpCodes.h"
#include "Program.h"

std::vector<uint8_t> parseByteCodeString(const std::string &str) {
    const char *buff = str.c_str();
    if (buff[1] == 'x')
        buff += 2; // Eat '0x'

    char scratch[3] = {};

    std::vector<uint8_t> rtn;
    while (*buff) {
        scratch[0] = buff[0];
        scratch[1] = buff[1];

        rtn.push_back(strtol(scratch, 0, 16));
        buff += 2;
    }

    return rtn;
}

std::string readFile(std::ifstream &fs) {
    std::string rtn, line;
    while (std::getline(fs, line))
        rtn += line;
    return rtn;
}

void createOutDir(const std::string& dir, const Program &program) {
    mkdir(dir.c_str(), S_IRWXU);

    {
        std::ofstream fs(dir + "/disassembly.txt");
        assert(fs);
        fs << DisassemReport(program, false, false);
    }

    {
        std::ofstream fs(dir + "/disassembly.full.txt");
        fs << DisassemReport(program, true, true);
    }

    {
        std::ofstream fs(dir + "/stackUsage.txt");
        fs << PsuedoStackReport(program);
    }

    size_t i = 0;
    for(auto& cc : program.createdContracts) {
        if(!cc) continue;

        std::stringstream ss;
        ss << "creates." << i++;
        createOutDir(dir + "/" + ss.str(), *cc);
    }

}

#define COMMAND_LINE_FLAGS \
XX(all) \
XX(outdir)

#define XX(name) bool name = false;

COMMAND_LINE_FLAGS

void createOutDir(const Program &program);

#undef XX

int main(int argc, const char **argv) {
    size_t farg = 1;
    for (size_t i = 1; i < argc; i++) {
        std::string arg = argv[i];
#define XX(name) if(arg == "--"#name) name = true;
        COMMAND_LINE_FLAGS
#undef XX

        if (arg[0] != '-')
            farg = i;
    }

    std::string fileName = argv[farg];
    std::ifstream f(fileName);
    auto bc = parseByteCodeString(readFile(f));

    Program p(bc);

    if(outdir) {
        auto dirName = fileName;
        auto pos = dirName.find_last_of('.');
        if(pos == std::string::npos) {
            dirName = "_" + dirName;
        } else {
            while(dirName.size() >= pos) {
                dirName.pop_back();
            }
        }

        createOutDir(dirName, p);
    } else {
        p.print(all, all);
    }

    return 0;
}

