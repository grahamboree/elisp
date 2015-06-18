//
//  util.cpp
//  elisp
//
//  Created by Graham Pentheny on 12/22/14.
//  Copyright (c) 2014 Graham Pentheny. All rights reserved.
//

#include <stdio.h>

#include "util.h"

namespace elisp {
    inline void die(string message) { throw std::logic_error(message); }
    template<typename T> void trueOrDie(T condition, string message) { if (!condition) die(message); }
    
    inline void replaceAll(string& str, const string& from, const string& to) {
        string::size_type pos = 0;
        while((pos = str.find(from, pos)) != string::npos) {
            str.replace(pos, from.length(), to);
            pos += to.length();
        }
    }
    
    bool isNumber(string inValue) {
        string::const_iterator it = inValue.begin();
        bool hasRadix = false;
        bool hasDigit = false;
        
        if (!inValue.empty() && ((hasDigit = (isdigit(*it) != 0)) || *it == '-')) {
            ++it;
            for (;it != inValue.end(); ++it) {
                bool digit = (isdigit(*it) != 0);
                hasDigit = hasDigit || digit;
                if (!(digit || (!hasRadix && (hasRadix = (*it == '.')))))
                    break;
            }
            return hasDigit && it == inValue.end();
        }
        return false;
    }
}