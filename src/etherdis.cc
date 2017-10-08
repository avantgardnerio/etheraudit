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

int main(int argc, const char**argv) {
  std::ifstream f(argv[1]);

  auto bc = parseByteCodeString(readFile(f));
  //printByteCode(bc);

  Program p(bc);
  p.print();
  auto pt = p.Instructions().find(70)->second.get();
  return 0;
}
