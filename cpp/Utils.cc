#include <sstream>
#include <algorithm>

#include "Utils.h"

std::string toString(const std::vector<uint8_t> &data) {
    std::stringstream ss;

    bool isFirst = true;
    for(auto& d : data) {
        if(!isFirst)
            ss << " ";
        ss.width(2);
        ss.fill('0');
        ss << std::hex << (uint32_t)d;
        isFirst = false;
    }
    return ss.str();
}

std::vector<uint8_t> getVecFromInt64(int64_t _v) {
    uint64_t v = static_cast<uint64_t>(_v);
    std::vector<uint8_t> rtn;
    if(_v < 0)
        for(size_t i = 0;i < 24;i++)
            rtn.push_back(0xff);

    while(v || rtn.empty()) {
        rtn.push_back(v & 0xff);
        v = v >> 8;
    }
    std::reverse(rtn.begin(), rtn.end());
    return rtn;
}

bool getInt64FromVec(const std::vector<uint8_t> &data, int64_t *rtn) {
    if(data.size() > 8)
        return false;

    int64_t v = 0;
    for(auto& d : data) {
        v = v << 8;
        v += d;
    }
    if(rtn)
        *rtn = v;

    return true;
}
