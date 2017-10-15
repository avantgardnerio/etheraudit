#pragma once

#include <stdlib.h>
#include <string>
#include <vector>

std::string toString(const std::vector<uint8_t>& data);

std::vector<uint8_t> getVecFromInt64(int64_t _v);

bool getInt64FromVec(const std::vector<uint8_t>& data, int64_t *rtn);
