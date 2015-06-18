//
//  util.h
//  elisp
//
//  Created by Graham Pentheny on 12/21/14.
//  Copyright (c) 2014 Graham Pentheny. All rights reserved.
//

#pragma once

#include <string>
#include <exception>

namespace elisp {
    using std::string;
    
    // Assertion functions
    void die(string message);
    template<typename T> void trueOrDie(T condition, string message);
    
    /**
     * Replaces all occurances of \p from with \p to in \p str
     */
    inline void replaceAll(string& str, const string& from, const string& to);
    
    /**
     * Returns \c true if \p inValue is a string representation of a number, \c false otherwise.
     */
    bool isNumber(string inValue);
}