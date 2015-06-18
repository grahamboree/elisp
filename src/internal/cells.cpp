//
//  cells.cpp
//  elisp
//
//  Created by Graham Pentheny on 12/21/14.
//  Copyright (c) 2014 Graham Pentheny. All rights reserved.
//

#include <stdio.h>

#include "util.h"
#include "cells.h"

namespace elisp {
    shared_ptr<cons_cell> empty_list = nullptr;
    
    /**
     * Converts a Cell to a bool.  Handles nullptr which is used to
     * indicate an empty list.
     */
    inline bool cell_to_bool(Cell cell) {
        return (cell != empty_list and (cell->GetType() != kCellType_bool || static_cast<bool_cell*>(cell.get())->GetValue()));
    }
    
    /**
     * A helper that creates a lisp list given a vector of the list's contents.
     *
     * A convenient use-case:
     * cons_cell* cons_cell = makeList({ new symbol_cell("+"), new number_cell(1), new number_cell(2)});
     */
    shared_ptr<cons_cell> makeList(vector<Cell> list) {
        return std::make_shared<cons_cell>(list);
    }
    
    int listLength(shared_ptr<cons_cell> list) {
        // TODO
        return 0;
        //return std::distance(list->begin(), list->end());
    }
    
    inline std::ostream& operator << (std::ostream& os, Cell obj) {
        return (os << (obj ? static_cast<string>(*obj) : "'()"));
    }
    
    inline number_cell::operator string() {
        if (valueString.empty()) {
            std::ostringstream ss;
            ss << ((value == (int)value) ? (int)value : value);
            return ss.str();
        }
        return valueString;
    }
    
    inline cons_cell::cons_cell(Cell inCar, Cell inCdr)
    : cell_t(kCellType_cons)
    , car(inCar)
    , cdr(inCdr)
    {
    }
    
    cons_cell::cons_cell(vector<Cell> inCells)
    : cell_t(kCellType_cons)
    {
        vector<Cell>::const_reverse_iterator iter;
        for (iter = inCells.rbegin(); iter != (inCells.rend() - 1); ++iter) {
            cdr = std::make_shared<cons_cell>(*iter, cdr);
        }
    }
    
    cons_cell::operator string() {
        std::ostringstream ss;
        auto consCell = shared_from_this();
        ss << "(";
        bool addSpace = false;
        while (true) {
            if (addSpace)
                ss << " ";
            addSpace = true;
            ss << consCell->car;
            if (!consCell->cdr) {
                break;
            } else if (consCell->cdr->GetType() == kCellType_cons) {
                consCell = std::static_pointer_cast<cons_cell>(consCell->cdr);
            } else {
                ss << " . " << consCell->cdr;
                break;
            }
        }
        ss << ")";
        return ss.str();
    }
    
    inline cons_cell::iterator::iterator(shared_ptr<cons_cell> startCell) : currentCell(startCell) {}
    
    inline cons_cell::iterator& cons_cell::iterator::operator++() {
        trueOrDie(currentCell->cdr == empty_list or currentCell->cdr->GetType() == kCellType_cons,
                  "Attempting to iterate through a cons-list that does not contain a cons cell in the cdr position.");
        currentCell = std::static_pointer_cast<cons_cell>(currentCell->cdr);
        return (*this);
    }
    
    inline cons_cell::iterator cons_cell::iterator::operator++(int) {
        iterator temp = *this;
        trueOrDie(currentCell->cdr == empty_list or currentCell->cdr->GetType() == kCellType_cons,
                  "Attempting to iterate through a cons-list that does not contain a cons cell in the cdr position.");
        currentCell = std::static_pointer_cast<cons_cell>(currentCell->cdr);
        return temp;
    }
    
    inline bool cons_cell::iterator::operator==(const iterator& other) { return currentCell == other.currentCell; }
    inline bool cons_cell::iterator::operator!=(const iterator& other) { return !((*this) == other); }
    inline Cell cons_cell::iterator::operator *() { return currentCell ? currentCell->GetCar() : nullptr; }
    
    inline lambda_cell::lambda_cell(Env outerEnv)
    : cell_t(kCellType_lambda)
    , env(outerEnv)
    , mVarargsName(nullptr)
    {
    }
    
    inline lambda_cell::lambda_cell(Env outerEnv,
                                    vector<shared_ptr<symbol_cell>>&& inParameters,
                                    vector<Cell>&& inBodyExpressions,
                                    shared_ptr<symbol_cell>&& inVarargsName)
    : cell_t(kCellType_lambda)
    , env(outerEnv)
    , mParameters(inParameters)
    , mBodyExpressions(inBodyExpressions)
    , mVarargsName(inVarargsName)
    {
    }
    
    Cell lambda_cell::eval(shared_ptr<cons_cell> args, Env currentEnv) {
        // TODO
        /*
        Env newEnv = std::make_shared<Environment>(env);
        
        // Match the arguments to the parameters.
        auto it = args->begin();
        for (auto paramID : mParameters) {
            trueOrDie(it != args->end(), "insufficient arguments provided to function");
            newEnv->mSymbolMap[paramID->GetIdentifier()] = currentEnv->eval(args->GetCar());
            ++it;
        }
        
        // Either store the rest of the arguments in the variadic name,
        // or error out that there were too many.
        if (mVarargsName) {
            vector<Cell> varargs;
            for (;it != args->end();++it)
                varargs.push_back(currentEnv->eval(*it));
            newEnv->mSymbolMap[mVarargsName->GetIdentifier()] = makeList(varargs);
        } else if (it != args->end()) {
            die("Too many arguments specified to lambda.");
        }
        
        // Evaluate the body expressions with the new environment.  Return the result of the last body expression.
        Cell returnVal;
        for (auto bodyExpr : mBodyExpressions)
            returnVal = newEnv->eval(bodyExpr);
        
        return returnVal;
        */
        return nullptr;
    }
    
    lambda_cell::operator string() {
        std::ostringstream ss;
        ss << "(lambda (";
        
        // parameters
        if (mVarargsName and mParameters.empty()) {
            ss << mVarargsName;
        } else {
            bool addspace = false;
            for (auto param : mParameters) {
                if (addspace)
                    ss << " ";
                else
                    addspace = true;
                ss << param;
            }
            if (mVarargsName)
                ss << " . " << mVarargsName;
            
            ss << ") ";
        }
        
        // body expressions
        bool addspace = false;
        for (auto bodyExpr : mBodyExpressions) {
            if (addspace)
                ss << " ";
            else
                addspace = true;
            ss << bodyExpr;
        }
        ss << ")";
        
        return ss.str();
    }
    
    template<typename T>
    inline bool_cell::bool_cell(T inValue)
    : cell_t(kCellType_bool)
    , value(inValue)
    {
    }
    
    inline bool_cell::bool_cell(bool inValue)
    : cell_t(kCellType_bool)
    , value(inValue)
    {
    }
    
    inline bool_cell::operator string() {
        return value ? "#t" : "#f";
    }
    
    inline number_cell::number_cell(double inValue)
    :cell_t(kCellType_number)
    , value(inValue)
    {
    }
    
    inline char_cell::char_cell(char inValue)
    :cell_t(kCellType_char)
    , value(inValue)
    {
    }
    
    inline char_cell::operator string() {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
    
    inline proc_cell::proc_cell(std::function<Cell(shared_ptr<cons_cell>, Env)> procedure)
    : cell_t(kCellType_procedure)
    , mProcedure(procedure)
    {
    }
    
    inline Cell proc_cell::evalProc(shared_ptr<cons_cell> args, Env env) {
        return mProcedure(args, env);
    };
    
    inline proc_cell::operator string() {
        return "#procedure";
    }
}