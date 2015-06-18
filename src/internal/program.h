//
//  Program.h
//  elisp
//
//  Created by Graham Pentheny on 12/21/14.
//  Copyright (c) 2014 Graham Pentheny. All rights reserved.
//

#pragma once

namespace elisp {
    using std::string;
    using std::vector;
    
    /** Program
     * Represents a discrete execution environment for elisp.
     */
    class Program {
    public:
        Program();
        
        /// Eval a string of code and give the result as a string.
        string runCode(string inCode);
        
        /// Given a stream, read and eval the code read from the stream.
        string runCode(TokenStream& stream);
        
        /// Read eval print loop.
        void repl(string prompt = "elisp> ");
        
        static Cell 		atom(const std::string& token);
        static vector<Cell> read(TokenStream& stream);
        static vector<Cell> read(string s);
        static string 		to_string(Cell exp);
        static Cell			expand(Cell x, bool topLevel = false);
        
    private:
        Env global_env;
    };
    
    
    /**
     * Given a string token, creates the atom it represents
     */
    Cell Program::atom(const string& token) {
        if (token[0] == '#') {
            const auto& boolid = token[1];
            bool val = (boolid == 't' || boolid == 'T');
            trueOrDie((val || boolid == 'f' || boolid == 'F') && token.size() == 2, "Unknown identifier " + token);
            return std::static_pointer_cast<cell_t>(std::make_shared<bool_cell>(val));
        } else if (token[0] == '"') {
            return std::static_pointer_cast<cell_t>(std::make_shared<string_cell>(token));
        } else if (isNumber(token)) {
            std::istringstream iss(token);
            double value = 0.0;
            iss >> value;
            auto n = std::make_shared<number_cell>(value);
            n->SetValueString(token);
            return std::static_pointer_cast<cell_t>(n);
        }
        
        return std::static_pointer_cast<cell_t>(std::make_shared<symbol_cell>(token));
    }
    
    /**
     * Returns a list of top-level expressions.
     */
    std::vector<Cell> Program::read(TokenStream& stream) {
        std::vector<std::vector<Cell>> exprStack; // The current stack of nested list expressions.
        exprStack.emplace_back(); // top-level scope
        
        for (string token = stream.nextToken(); !token.empty(); token = stream.nextToken()) {
            if (token == "(") {
                // Push a new scope onto the stack.
                exprStack.emplace_back();
            } else if (token == ")") {
                // Pop the current scope off the stack and add it as a list to its parent scope.
                trueOrDie(exprStack.size() > 1, "Unexpected ) while reading");
                Cell listexpr = makeList(exprStack.back());
                exprStack.pop_back();
                exprStack.back().push_back(listexpr);
            } else {
                exprStack.back().push_back(atom(token));
            }
        }
        
        trueOrDie(exprStack.size() == 1, "Unexpected EOF while reading");
        return exprStack.back();
    }
    
    /**
     * Returns a list of top-level expressions.
     */
    inline std::vector<Cell> Program::read(string s) {
        std::istringstream iss(s);
        TokenStream tokStream(iss);
        return read(tokStream);
    }
    
    inline string Program::to_string(Cell exp) {
        if (exp == nullptr)
            return "'()";
        
        std::ostringstream ss;
        ss << exp;
        return ss.str();
    }
    
    inline Program::Program() {
        global_env = std::make_shared<Environment>();
        add_globals(global_env);
    }
    
    /// Eval a string of code and give the result as a string.
    inline string Program::runCode(string inCode) {
        std::istringstream iss(inCode);
        TokenStream tokStream(iss);
        return runCode(tokStream);
    }
    
    /// Given a stream, read and eval the code read from the stream.
    string Program::runCode(TokenStream& stream) {
        using std::cerr;
        using std::endl;
        
        try {
            Cell result = nullptr;
            for (auto expr : read(stream))
                result = global_env->eval(expr);
            return to_string(result);
        } catch (const std::logic_error& e) {
            // logic_error's are thrown for invalid code.
            cerr << "[ERROR]\t" << e.what() << endl;
        } catch (const std::exception& e) {
            // runtime_error's are internal errors at no fault of the user.
            cerr << endl << endl << "--[SYSTEM ERROR]--" << endl << endl << e.what() << endl << endl;
        } catch (...) {
            cerr << endl << endl << "--[SYSTEM ERROR]--" << endl << endl << "An unkown error occured" << endl << endl;
        }
        return "";
    }
    
    /// Read eval print loop.
    void Program::repl(string prompt) {
        using std::cout;
        using std::endl;
        
        while (true) {
            cout << prompt;
            
            string raw_input;
            if (!std::getline(std::cin, raw_input)) {
                cout << endl << endl;
                break;
            }
            
            if (raw_input.empty() or raw_input.find_first_not_of(" \t") == string::npos)
                continue;
            cout << runCode(raw_input) << endl;
        }
    }
}