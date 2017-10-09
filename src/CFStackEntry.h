#pragma once

#include <vector>
#include <stdlib.h>

struct CFStackEntry {
    size_t idx;
    std::string label = "";
    bool isConstant = false;
    std::vector<uint8_t> constantValue;

    bool getConstantInt(int64_t* v);

    friend std::ostream &operator<<(std::ostream &os, const CFStackEntry &entry);

    bool operator==(const CFStackEntry &rhs) const;

    bool operator!=(const CFStackEntry &rhs) const;

    bool operator<(const CFStackEntry &rhs) const;

    bool operator>(const CFStackEntry &rhs) const;

    bool operator<=(const CFStackEntry &rhs) const;

    bool operator>=(const CFStackEntry &rhs) const;
};