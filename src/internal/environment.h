//
//  environment.h
//  elisp
//
//  Created by Graham Pentheny on 12/21/14.
//  Copyright (c) 2014 Graham Pentheny. All rights reserved.
//

#pragma once

namespace elisp {
    /** Environment
     * A symbol mapping environment.  Designed as a tree where each node
     * represents a scope which can override the parent scope bindings.
     */
    class Environment : public std::enable_shared_from_this<Environment> {
    public:
        Environment();
        Environment(Env inOuter);
        
        Env find(const string& var);
        Cell get(const string& var);
        Cell eval(Cell x);
        
    private:
        Env outer;
        
    public: // TODO This shouldn't be public.
        std::map<string, Cell> mSymbolMap;
    };
    inline Environment::Environment() {}
    inline Environment::Environment(Env inOuter) :outer(inOuter) {}
    
    inline Env Environment::find(const string& var) {
        if (mSymbolMap.find(var) != mSymbolMap.end())
            return shared_from_this();
        trueOrDie(outer, "Undefined symbol " + var);
        return outer->find(var);
    }
    
    inline Cell Environment::get(const string& var) {
        return mSymbolMap.find(var)->second;
    }
    
    Cell Environment::eval(Cell x) {
        trueOrDie(x != nullptr, "Missing procedure.  Original code was most likely (), which is illegal.");
        
        if (x->GetType() == kCellType_symbol) {
            // Symbol lookup in the current environment.
            const string& id = static_cast<symbol_cell*>(x.get())->GetIdentifier();
            return find(id)->get(id);
        } else if (x->GetType() == kCellType_cons) {
            // Function call
            cons_cell* listcell = static_cast<cons_cell*>(x.get());
            Cell callable = this->eval(listcell->GetCar());
            
            // If the first argument is a symbol, look it up in the current environment.
            if (callable->GetType() == kCellType_symbol) {
                string callableName = static_cast<symbol_cell*>(callable.get())->GetIdentifier();
                
                Env enclosingEnvironment = find(callableName);
                trueOrDie(enclosingEnvironment, "Undefined function: " + callableName);
                
                callable = enclosingEnvironment->get(callableName);
            }
            
            if (callable->GetType() == kCellType_procedure) {
                // Eval the procedure with the rest of the arguments.
                trueOrDie(listcell->GetCdr()->GetType() == kCellType_cons, "Cannot call a procedure with something that's not a cons-list.");
                return static_cast<proc_cell*>(callable.get())->evalProc(
                                                                         std::static_pointer_cast<cons_cell>(listcell->GetCdr()),
                                                                         shared_from_this());
            } else if (callable->GetType() == kCellType_lambda) {
                // Eval the lambda with the rest of the arguments.
                trueOrDie(listcell->GetCdr()->GetType() == kCellType_cons, "Cannot call a lambda with something that's not a cons-list.");
                return static_cast<lambda_cell*>(callable.get())->eval(
                                                                       std::static_pointer_cast<cons_cell>(listcell->GetCdr()),
                                                                       shared_from_this());
            }
            die("Expected procedure or lambda as first element in an sexpression.");
        }
        return x;
    }
}