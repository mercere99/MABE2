/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  AST.hpp
 *  @brief Manages Abstract Syntax Tree nodes for Emplode.
 *  @note Status: BETA
 */

#ifndef EMPLODE_AST_HPP
#define EMPLODE_AST_HPP

#include "emp/base/assert.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"

#include "Symbol.hpp"
#include "Symbol_Scope.hpp"
#include "Symbol_Object.hpp"
#include "SymbolTableBase.hpp"

namespace emplode {

  /// Base class for all AST Nodes.
  class ASTNode {
  protected:
    using symbol_ptr_t = emp::Ptr<Symbol>;
    using symbol_vector_t = emp::vector<symbol_ptr_t>;

    using node_ptr_t = emp::Ptr<ASTNode>;
    using node_vector_t = emp::vector<node_ptr_t>;

    node_ptr_t parent = nullptr;
    int line_id = -1;             // Line number of input file with error.

  public:
    ASTNode() { ; }
    virtual ~ASTNode() { ; }

    int GetLine() const { return line_id; }
    void SetLine(int in_line) { line_id = in_line; }

    virtual const std::string & GetName() const = 0;

    virtual bool IsNumeric() const { return false; } // Can node be represented as a number?
    virtual bool IsString() const { return false; }  // Can node be represented as a string?
    virtual bool HasValue() const { return false; }  // Does node have any value (vs internal block)
    virtual bool HasNumericReturn() const { return false; } // Is node function with numeric return?
    virtual bool HasStringReturn() const { return false; }  // Is node function with string return?

    virtual bool IsLeaf() const { return false; }
    virtual bool IsInternal() const { return false; }
    virtual bool IsBlock() const { return false; }

    virtual size_t GetNumChildren() const { return 0; }
    virtual node_ptr_t GetChild(size_t /* id */) { emp_assert(false); return nullptr; }
    node_ptr_t GetParent() { return parent; }
    void SetParent(node_ptr_t in_parent) { parent = in_parent; }
    virtual emp::Ptr<Symbol_Scope> GetScope() { return parent ? parent->GetScope() : nullptr; }
    virtual SymbolTableBase & GetSymbolTable() { return parent->GetSymbolTable(); }

    virtual symbol_ptr_t Process() = 0;

    virtual void Write(std::ostream & /* os */=std::cout,
                       const std::string & /* offset */="") const { }
  };

  /// An ASTNode representing an internal node.
  class ASTNode_Internal : public ASTNode {
  protected:
    std::string name;
    node_vector_t children;

  public:
    ASTNode_Internal(const std::string & _name="") : name (_name) { }
    ~ASTNode_Internal() { 
      for (auto child : children) child.Delete();
    }

    const std::string & GetName() const override { return name; }

    bool IsInternal() const override { return true; }

    size_t GetNumChildren() const override { return children.size(); }
    node_ptr_t GetChild(size_t id) override { return children[id]; }

    void AddChild(node_ptr_t child) {
      children.push_back(child);
      child->SetParent(this);
    }
  };

  /// An ASTNode representing a leaf in the tree (i.e., a variable or literal)
  class ASTNode_Leaf : public ASTNode {
  protected:
    symbol_ptr_t symbol_ptr;  ///< Pointer to Symbol at this leaf.
    bool own_symbol;          ///< Should this node be in charge of deleting the symbol?

  public:
    ASTNode_Leaf(symbol_ptr_t _ptr, int _line=-1)
      : symbol_ptr(_ptr), own_symbol(_ptr->IsTemporary())
    {
      symbol_ptr->SetTemporary(false); // If this symbol was temporary, it is now owned.
      line_id = _line;
    }
    ~ASTNode_Leaf() { if (own_symbol) symbol_ptr.Delete(); }

    const std::string & GetName() const override { return symbol_ptr->GetName(); }
    Symbol & GetSymbol() { return *symbol_ptr; }

    bool IsNumeric() const override { return symbol_ptr->IsNumeric(); }
    bool IsString() const override { return symbol_ptr->IsString(); }
    bool HasValue() const override { return true; }
    bool HasNumericReturn() const override { return symbol_ptr->HasNumericReturn(); }
    bool HasStringReturn() const override { return symbol_ptr->HasStringReturn(); }

    bool IsLeaf() const override { return true; }

    symbol_ptr_t Process() override { return symbol_ptr; };

    void Write(std::ostream & os, const std::string &) const override {
      // If this is a variable, print the variable name,
      std::string output = symbol_ptr->GetName();

      // If it is a literal, print the value.
      if (output == "") {
        output = symbol_ptr->AsString();

        // If the symbol is a string, convert it to a string literal.
        if (symbol_ptr->IsString()) output = emp::to_literal(output);
      }
      os << output;
    }
  };

  // Helper functions for making temporary leaves.
  emp::Ptr<ASTNode_Leaf> MakeTempLeaf(double val) {
    auto out_ptr = emp::NewPtr<Symbol_Var>("__Temp", val, "Temporary double", nullptr);
    out_ptr->SetTemporary();
    return emp::NewPtr<ASTNode_Leaf>(out_ptr);
  }

  emp::Ptr<ASTNode_Leaf> MakeTempLeaf(const std::string & val) {
    auto out_ptr = emp::NewPtr<Symbol_Var>("__Temp", val, "Temporary string", nullptr);
    out_ptr->SetTemporary();
    return emp::NewPtr<ASTNode_Leaf>(out_ptr);
  }

  class ASTNode_Block : public ASTNode_Internal {
  protected:
    emp::Ptr<Symbol_Scope> scope_ptr;
    emp::Ptr<SymbolTableBase> symbol_table = nullptr;

  public:
    ASTNode_Block(Symbol_Scope & in_scope, int in_line=-1) : scope_ptr(&in_scope) {
      line_id = in_line;
    }

    bool IsBlock() const override { return true; }

    emp::Ptr<Symbol_Scope> GetScope() override { return scope_ptr; }

    SymbolTableBase & GetSymbolTable() override {
      if (symbol_table) return *symbol_table;
      return parent->GetSymbolTable();
    }
    void SetSymbolTable(SymbolTableBase & _st) { symbol_table = &_st; }

    symbol_ptr_t Process() override {
      for (auto node : children) {
        symbol_ptr_t out = node->Process();
        if (out && out->IsTemporary()) out.Delete();
      }
      return nullptr;
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      for (auto child_ptr : children) {
        child_ptr->Write(os, offset+"  ");
        os << ";\n" << offset;
      }
    }
  };

  /// Unary mathematical operations.
  class ASTNode_Math1 : public ASTNode_Internal {
  protected:
    // A unary operator take in a double and returns another one.
    std::function< double(double) > fun;
  public:
    ASTNode_Math1(const std::string & name, int _line=-1) : ASTNode_Internal(name) { 
      line_id = _line;
    }

    bool IsNumeric() const override { return true; }
    bool HasValue() const override { return true; }

    void SetFun(std::function< double(double) > _fun) { fun = _fun; }

    symbol_ptr_t Process() override {
      emp_assert(children.size() == 1);
      symbol_ptr_t input_symbol = children[0]->Process();     // Process child to get input symbol
      double output_value = fun(input_symbol->AsDouble());    // Run the function to get ouput value
      if (input_symbol->IsTemporary()) input_symbol.Delete(); // If we are done with input; delete!
      return GetSymbolTable().MakeTempSymbol(output_value);
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      os << name;
      children[0]->Write(os, offset);
    }
  };

  /// Binary operations.
  template <typename RETURN_T, typename ARG1_T, typename ARG2_T>
  class ASTNode_Op2 : public ASTNode_Internal {
  protected:
    std::function< RETURN_T(ARG1_T, ARG2_T) > fun;
  public:
    ASTNode_Op2(const std::string & name, int _line=-1) : ASTNode_Internal(name) {
      line_id = _line;
    }

    bool IsNumeric() const override { return std::is_same<RETURN_T, double>(); }
    bool IsString() const override { return std::is_same<RETURN_T, std::string>(); }
    bool HasValue() const override { return true; }

    void SetFun(std::function< RETURN_T(ARG1_T, ARG2_T) > _fun) { fun = _fun; }

    symbol_ptr_t Process() override {
      emp_assert(children.size() == 2);
      symbol_ptr_t in1 = children[0]->Process();               // Process 1st child to input symbol
      symbol_ptr_t in2 = children[1]->Process();               // Process 2nd child to input symbol
      auto out_val = fun(in1->As<ARG1_T>(), in2->As<ARG2_T>()); // Run function; get ouput
      if (in1->IsTemporary()) in1.Delete();                   // If we are done with in1; delete!
      if (in2->IsTemporary()) in2.Delete();                   // If we are done with in2; delete!
      return GetSymbolTable().MakeTempSymbol(out_val);
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      children[0]->Write(os, offset);
      os << " " << name << " ";
      children[1]->Write(os, offset);
    }
  };

  using ASTNode_Math2 = ASTNode_Op2<double,double,double>;


  class ASTNode_Assign : public ASTNode_Internal {
  public:
    ASTNode_Assign(node_ptr_t lhs, node_ptr_t rhs, int _line=-1) {
      AddChild(lhs);
      AddChild(rhs);
      line_id = _line;
    }

    bool IsNumeric() const override { return children[0]->IsNumeric(); }
    bool IsString() const override { return children[0]->IsString(); }
    bool HasValue() const override { return true; }
    bool HasNumericReturn() const override { return children[0]->HasNumericReturn(); }
    bool HasStringReturn() const override { return children[0]->HasStringReturn(); }

    symbol_ptr_t Process() override {
      emp_assert(children.size() == 2);
      symbol_ptr_t lhs = children[0]->Process();  // Determine the left-hand-side value.
      symbol_ptr_t rhs = children[1]->Process();  // Determine the right-hand-side value.

      // @CAO Should make sure that lhs is properly assignable.
      bool success = lhs->CopyValue(*rhs);
      if (!success) {
        std::cerr << "Error: copy to '" << lhs->GetName() << "' failed" << std::endl;
        exit(1);
      }
      if (rhs->IsTemporary()) rhs.Delete();
      return lhs;
    }
  };

  class ASTNode_If : public ASTNode_Internal {
  public:
    ASTNode_If(node_ptr_t test, node_ptr_t true_node, node_ptr_t else_node, int _line=-1) {
      AddChild(test);
      AddChild(true_node);
      if (else_node) AddChild(else_node);
      line_id = _line;
    }

    symbol_ptr_t Process() override {
      symbol_ptr_t test = children[0]->Process();  // Determine the left-hand-side value.

      // Handle TRUE
      if (test->AsDouble() != 0.0) {
        symbol_ptr_t result = children[1]->Process();
        if (result && result->IsTemporary()) result.Delete();
      }

      // Handle FALSE
      else if (children.size() > 2) {
        symbol_ptr_t result = children[2]->Process();
        if (result && result->IsTemporary()) result.Delete();
      }
      
      if (test->IsTemporary()) test.Delete();
      return nullptr;
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      os << "IF (";
      children[0]->Write(os, offset);
      os << ") ";
      children[1]->Write(os, offset);
      if (children.size() > 2) {
        os << "\n" << offset << "ELSE ";
        children[2]->Write(os, offset);
      }
    }
  };

  class ASTNode_Call : public ASTNode_Internal {
  public:
    ASTNode_Call(node_ptr_t fun, const node_vector_t & args, int _line=-1) {
      AddChild(fun);
      for (auto arg : args) AddChild(arg);
      line_id = _line;
    }

    bool IsNumeric() const override { return children[0]->HasNumericReturn(); }
    bool IsString() const override { return children[0]->HasStringReturn(); }
    bool HasValue() const override { return true; }
    // @CAO Technically, one function can return another, so we should check
    // HasNumericReturn() and HasStringReturn() on return values... but hard to implement.

    symbol_ptr_t Process() override {
      emp_assert(children.size() >= 1);
      symbol_ptr_t fun = children[0]->Process();

      // Collect all arguments and call
      symbol_vector_t args;
      for (size_t i = 1; i < children.size(); i++) {
        args.push_back(children[i]->Process());
      }
      symbol_ptr_t result = fun->Call(args);

      // Cleanup and return
      for (auto arg : args) if (arg->IsTemporary()) arg.Delete();
      return result;
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      children[0]->Write(os, offset);  // Function name
      os << "(";
      for (size_t i=1; i < children.size(); i++) {
        if (i>1) os << ", ";
        children[i]->Write(os, offset);
      }
      os << ")";
    }
  };

  class ASTNode_Event : public ASTNode_Internal {
  protected:
    using setup_fun_t = std::function<void(node_ptr_t, const symbol_vector_t &)>;
    setup_fun_t setup_event;

  public:
    ASTNode_Event(
      const std::string & event_name,
      node_ptr_t action,
      const node_vector_t & args,
      setup_fun_t in_fun,
      int _line=-1
    )
     : ASTNode_Internal(event_name), setup_event(in_fun)
    {
      AddChild(action);
      for (auto arg : args) AddChild(arg);
      line_id = _line;
    }

    symbol_ptr_t Process() override {
      emp_assert(children.size() >= 1);
      symbol_vector_t arg_entries;
      for (size_t id = 1; id < children.size(); id++) {
        arg_entries.push_back( children[id]->Process() );
      }
      setup_event(children[0], arg_entries);
      return nullptr;
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      os << "@" << GetName() << "(";
      for (size_t i = 1; i < children.size(); i++) {
        if (i>1) os << ", ";
        children[i]->Write(os, offset);
      }
      os << ") ";
      children[0]->Write(os, offset);  // Action.
    }
  };

}

#endif
