

#include "Program.h"
#include <iostream>
#include <set>
#include "CFStackEntry.h"
#include "Utils.h"

bool CFStackEntry::getConstantInt(int64_t *v) {
    if(!isConstant)
        return false;

    return getInt64FromVec(constantValue, v);
}

std::ostream &operator<<(std::ostream &os, const CFStackEntry &entry) {
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

bool CFStackEntry::operator==(const CFStackEntry &rhs) const {
    return idx == rhs.idx &&
           label == rhs.label &&
           isConstant == rhs.isConstant &&
           constantValue == rhs.constantValue;
}

bool CFStackEntry::operator!=(const CFStackEntry &rhs) const {
    return !(rhs == *this);
}

bool CFStackEntry::operator<(const CFStackEntry &rhs) const {
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

bool CFStackEntry::operator>(const CFStackEntry &rhs) const {
    return rhs < *this;
}

bool CFStackEntry::operator<=(const CFStackEntry &rhs) const {
    return !(rhs < *this);
}

bool CFStackEntry::operator>=(const CFStackEntry &rhs) const {
    return !(*this < rhs);
}