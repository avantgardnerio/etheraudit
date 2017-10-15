#pragma once

#include <vector>
#include <stdlib.h>

struct CFExpression {
    size_t idx;
    std::string label = "";
    bool isConstant = false;
    std::vector<uint8_t> constantValue;

    bool isSymbolic() const;
    bool getConstantInt(int64_t* v);

    friend std::ostream &operator<<(std::ostream &os, const CFExpression &entry);

    bool operator==(const CFExpression &rhs) const;

    bool operator!=(const CFExpression &rhs) const;

    bool operator<(const CFExpression &rhs) const;

    bool operator>(const CFExpression &rhs) const;

    bool operator<=(const CFExpression &rhs) const;

    bool operator>=(const CFExpression &rhs) const;
};