//
//  tokenstream.h
//  elisp
//
//  Created by Graham Pentheny on 12/21/14.
//  Copyright (c) 2014 Graham Pentheny. All rights reserved.
//

#pragma once

namespace elisp {
    /*--------------------------------------------------------------------------------*/
    /**
     * TokenStream
     * A wrapper around std::istream that allows you to grab individual tokens lazily.
     */
    class TokenStream {
    public:
        TokenStream(std::istream& stream);
        
        /** Gets the next token, or returns the empty string to indicate EOF */
        std::string nextToken();
    private:
        static const std::regex reg;
        std::istream& is;
        std::string line;
    };
}