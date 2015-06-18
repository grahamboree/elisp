//
//  cells.h
//  elisp
//
//  Created by Graham Pentheny on 12/21/14.
//  Copyright (c) 2014 Graham Pentheny. All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <sstream>

namespace elisp {
    using std::string;
    using std::shared_ptr;
    using std::vector;
    
    class Environment;
    typedef shared_ptr<Environment> Env;
    
    /// Data types in elisp
    enum eCellType {
        kCellType_bool,
        kCellType_number,
        kCellType_char,
        kCellType_string,
        kCellType_symbol,
        kCellType_cons,
        kCellType_pair,
        kCellType_vector,
        kCellType_procedure,
        kCellType_lambda
    };
    
    /// Cell type base class
    class cell_t {
    public:
        virtual ~cell_t() {}
        
        /// Used by the writer to print the string representation of code.
        virtual string to_string() = 0;
        
        eCellType GetType() { return type; }
    protected:
        cell_t(eCellType inType) :type(inType) {}
        eCellType type;
    };
    
    typedef shared_ptr<cell_t> Cell;
    
    class bool_cell : public cell_t {
    public:
        template<typename T>
        bool_cell(T inValue);
        bool_cell(bool inValue);
        virtual ~bool_cell() {}
        
        virtual string to_string() override;
        
        bool GetValue() { return value; }
    private:
        bool value;
    };
    
    class number_cell : public cell_t {
    public:
        number_cell(double inValue);
        virtual ~number_cell() {}
        
        virtual string to_string() override;
        
        double GetValue() { return value; }
        void SetValueString(string valuestr) { valueString = valuestr; }
    private:
        double value;
        string valueString;
    };
    
    class char_cell : public cell_t {
    public:
        char_cell(char inValue);
        virtual ~char_cell() {}
        
        virtual string to_string() override;
    private:
        char value;
    };
    
    class string_cell : public cell_t {
    public:
        string_cell(string inVal) :cell_t(kCellType_string), value(inVal) {}
        virtual ~string_cell() {}
        
        virtual string to_string() override { return value; }
    private:
        string value;
    };
    
    class symbol_cell : public cell_t {
    public:
        symbol_cell(string id) :cell_t(kCellType_symbol), identifier(id) {}
        virtual ~symbol_cell() {}
        
        virtual string to_string() override { return identifier; }
        
        string GetIdentifier() { return identifier; }
    private:
        string identifier;
    };
    
    class cons_cell : public cell_t, public std::enable_shared_from_this<cons_cell> {
    public:
        class iterator {
            shared_ptr<cons_cell> currentCell;
        public:
            typedef int difference_type;
            
            iterator(shared_ptr<cons_cell> startCell);
            
            iterator& operator++();
            iterator operator++(int);
            bool operator==(const iterator& other);
            bool operator!=(const iterator& other);
            
            Cell operator *();
        };
        
        cons_cell(Cell inCar, Cell inCdr);
        cons_cell(vector<Cell> inCells);
        virtual ~cons_cell() {}
        
        virtual string to_string() override;
        
        Cell GetCar() { return car; }
        void SetCar(Cell newCar) { car = newCar; }
        
        Cell GetCdr() { return cdr; }
        void SetCdr(Cell newCdr) { cdr = newCdr; }
        
        iterator begin()  { return iterator(shared_from_this()); }
        iterator end()  { return iterator(nullptr); }
        
    private:
        Cell car;
        Cell cdr;
    };
    
    /**
     * Wrapper for a built-in procedure not expressed in the langauge.
     */
    class proc_cell : public cell_t {
    public:
        proc_cell(std::function<Cell(shared_ptr<cons_cell>, Env)> procedure);
        virtual ~proc_cell() {}
        
        virtual string to_string() override;
        
        Cell evalProc(shared_ptr<cons_cell> args, Env env);
    private:
        std::function<Cell(shared_ptr<cons_cell>, Env)> mProcedure;
    };
    
    /**
     * Structure of a function defined in the langauge.
     */
    class lambda_cell : public cell_t {
    public:
        lambda_cell(Env outerEnv);
        lambda_cell(Env outerEnv, vector<shared_ptr<symbol_cell>>&& inParameters, vector<Cell>&& inBodyExpressions, shared_ptr<symbol_cell>&& inVarargsName);
        virtual ~lambda_cell() {}
        
        virtual Cell eval(shared_ptr<cons_cell> args, Env currentEnv);
        virtual string to_string() override;
        
    private:
        Env env;
        vector<shared_ptr<symbol_cell>> mParameters; // 0 or more arguments. 
        vector<Cell> 					mBodyExpressions; // 1 or more body statements. 
        shared_ptr<symbol_cell> 		mVarargsName; // name of the varargs parameter if there is one.
    };
}