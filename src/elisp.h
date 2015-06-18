//
//  elisp.h
//  Elisp include header
//
//  Created by Graham Pentheny on 12/21/14.
//  Copyright (c) 2014 Graham Pentheny. All rights reserved.
//

#pragma once

// STL dependencies
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <sstream>
#include <vector>

namespace elisp {
    using std::string;
    using std::vector;
    using std::shared_ptr;
    
    class TokenStream;
    
    class Environment;
    typedef shared_ptr<Environment> Env;
}

#include "util.h"
#include "cells.h"
/*
#include "tokenstream.h"
#include "program.h"
#include "environment.h"
#include "elispImpl.h"
*/
