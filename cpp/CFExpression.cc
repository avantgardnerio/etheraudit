

#include "Program.h"
#include <iostream>
#include <set>
#include "CFExpression.h"
#include "Utils.h"

bool CFExpression::getConstantInt(int64_t *v) {
    if(!isConstant)
        return false;

    return getInt64FromVec(constantValue, v);
}

std::ostream &operator<<(std::ostream &os, const CFExpression &entry) {
    if(entry.isConstant) {
        os << "{" << toString(entry.constantValue) << "}";
    } else {

        if(!entry.label.empty()) {
            os << "<" << entry.label << ".";
        } else {
            os << "<#";
        }
        os << std::dec << entry.idx << ">";
    }
    return os;
}

bool CFExpression::operator==(const CFExpression &rhs) const {
    return idx == rhs.idx &&
           label == rhs.label &&
           isConstant == rhs.isConstant &&
           constantValue == rhs.constantValue;
}

bool CFExpression::operator!=(const CFExpression &rhs) const {
    return !(rhs == *this);
}

bool CFExpression::operator<(const CFExpression &rhs) const {
    if (idx < rhs.idx)
        return true;
    if (rhs.idx < idx)
        return false;
    if (label < rhs.label)
        return true;
    if (rhs.label < label)
        return false;
    if (isConstant < rhs.isConstant)
        return true;
    if (rhs.isConstant < isConstant)
        return false;
    return constantValue < rhs.constantValue;
}

bool CFExpression::operator>(const CFExpression &rhs) const {
    return rhs < *this;
}

bool CFExpression::operator<=(const CFExpression &rhs) const {
    return !(rhs < *this);
}

bool CFExpression::operator>=(const CFExpression &rhs) const {
    return !(*this < rhs);
}

bool CFExpression::isSymbolic() const {
    if(isConstant)
        return false;
    if(!label.empty())
        return false;
    return true;
}
