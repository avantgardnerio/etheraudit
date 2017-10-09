#include <fstream>
#include <vector>
#include <assert.h>
#include <iostream>
#include <memory>
#include <map>
#include <sstream>
#include <algorithm>

#include "OpCodes.h"
#include "Program.h"

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

#define COMMAND_LINE_FLAGS \
XX(all)

#define XX(name) bool name = false;
COMMAND_LINE_FLAGS
#undef XX

int main(int argc, const char**argv) {
  size_t farg = 1;
  for(size_t i = 1;i < argc;i++) {
      std::string arg = argv[i];
#define XX(name) if(arg == "--"#name) name = true;
      COMMAND_LINE_FLAGS
#undef XX

      if(arg[0] != '-')
          farg = i;
  }

    std::ifstream f(argv[farg]);
    auto bc = parseByteCodeString(readFile(f));
  //printByteCode(bc);

  Program p(bc);
    p.print(all, all);
  auto pt = p.Instructions().find(70)->second.get();
  return 0;
}
