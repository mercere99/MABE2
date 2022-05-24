/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2022.
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

    // Helper alternatives for Process()

    /// Run process and clean up any returned symbols automatically, as needed.
    void ProcessVoid() {
      symbol_ptr_t out = Process();
      if (out && out->IsTemporary()) out.Delete();
    }

    /// Run process, convert the return value to a double, and clean up the symbol if needed.
    template <typename T>
    T ProcessAs() {
      symbol_ptr_t symbol_ptr = Process();                // Run process, collecting the result.
      if (!symbol_ptr) return T();                        // Any non value will return a zero.
      T result = symbol_ptr->As<T>();                     // Convert the result to the return type.
      if (symbol_ptr->IsTemporary()) symbol_ptr.Delete(); // If we are done with input; delete the symbol!
      return result;
    }

    virtual void PrintAST(std::ostream & os=std::cout, size_t indent=0) = 0;
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

    symbol_ptr_t Process() override { 
      #ifndef NDEBUG
      emp::notify::Verbose(
        "Emplode::AST",
        "AST: Calling leaf '", symbol_ptr ? symbol_ptr->AsString() : std::string("[null]")
      );
      #endif
      return symbol_ptr;
    };

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

    void PrintAST(std::ostream & os=std::cout, size_t indent=0) override {
      for (size_t i = 0; i < indent; ++i) os << " ";
      os << "ASTNode_Leaf : " << symbol_ptr->DebugString() << std::endl;
    }
  };

  // Helper functions for making temporary leaves.
  emp::Ptr<ASTNode_Leaf> MakeTempLeaf(double val, int line_id=-1) {
    auto symbol_ptr = emp::NewPtr<Symbol_Var>("__Temp", val, "Temporary double", nullptr);
    symbol_ptr->SetTemporary();
    return emp::NewPtr<ASTNode_Leaf>(symbol_ptr, line_id);
  }

  emp::Ptr<ASTNode_Leaf> MakeTempLeaf(const std::string & val, int line_id=-1) {
    auto symbol_ptr = emp::NewPtr<Symbol_Var>("__Temp", val, "Temporary string", nullptr);
    symbol_ptr->SetTemporary();
    return emp::NewPtr<ASTNode_Leaf>(symbol_ptr, line_id);
  }

  emp::Ptr<ASTNode_Leaf> MakeBreakLeaf(int line_id=-1) {
      static Symbol_Special break_symbol(Symbol_Special::BREAK);
      return emp::NewPtr<ASTNode_Leaf>(&break_symbol, line_id);
  }

  emp::Ptr<ASTNode_Leaf> MakeContinueLeaf(int line_id=-1) {
      static Symbol_Special continue_symbol(Symbol_Special::CONTINUE);
      return emp::NewPtr<ASTNode_Leaf>(&continue_symbol, line_id);
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
      #ifndef NDEBUG
      emp::notify::Verbose(
        "Emplode::AST",
        "AST: Processing BLOCK"
      );
      #endif

      for (auto node : children) {
        symbol_ptr_t out = node->Process();                  // Process this line.
        if (!out) continue;                                  // No return symbol?  Keep going!
        if (out->IsBreak() || out->IsContinue()) return out; // Propagate a break or continue
        if (out->IsTemporary()) out.Delete();                // Clean up anything else, if needed
      }
      return nullptr;
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      for (auto child_ptr : children) {
        child_ptr->Write(os, offset+"  ");
        os << ";\n" << offset;
      }
    }

    void PrintAST(std::ostream & os=std::cout, size_t indent=0) override {
      for (size_t i = 0; i < indent; ++i) os << " ";
      os << "ASTNode_Block: " << children.size() << " lines." << std::endl;
      for (auto child_ptr : children) child_ptr->PrintAST(os, indent+2);
    }
  };

  /// Unary operations.
  class ASTNode_Op1 : public ASTNode_Internal {
  protected:
    // A unary operator take in a double and returns another one.
    std::function< double(double) > fun;
  public:
    ASTNode_Op1(const std::string & name, int _line=-1) : ASTNode_Internal(name) { 
      line_id = _line;
    }

    bool IsNumeric() const override { return true; }
    bool HasValue() const override { return true; }

    void SetFun(std::function< double(double) > _fun) { fun = _fun; }

    symbol_ptr_t Process() override {
      emp_assert(children.size() == 1);
      #ifndef NDEBUG
      emp::notify::Verbose(
        "Emplode::AST",
        "AST: Processing unary math: ", name
      );
      #endif
      double result = fun(children[0]->ProcessAs<double>());    // Run the function to get ouput value
      return GetSymbolTable().MakeTempSymbol(result);
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      os << name;
      children[0]->Write(os, offset);
    }

    void PrintAST(std::ostream & os=std::cout, size_t indent=0) override {
      for (size_t i = 0; i < indent; ++i) os << " ";
      os << "ASTNode_Op1: " << GetName() << std::endl;
      for (auto child : children) child->PrintAST(os, indent+2);
    }
  };

  /// Binary operations.
  class ASTNode_Op2 : public ASTNode_Internal {
  protected:
    std::function< emp::Datum(emp::Datum, emp::Datum) > fun;
  public:
    ASTNode_Op2(const std::string & name, int _line=-1) : ASTNode_Internal(name) {
      line_id = _line;
    }

    // Type is always linked to the first argument.
    bool IsNumeric() const override { return children[0]->IsNumeric(); }
    bool IsString() const override { return children[0]->IsString(); }
    bool HasValue() const override { return true; }

    void SetFun(std::function< emp::Datum(emp::Datum, emp::Datum) > _fun) { fun = _fun; }

    symbol_ptr_t Process() override {
      emp_assert(children.size() == 2);
      #ifndef NDEBUG
      emp::notify::Verbose(
        "Emplode::AST",
        "AST: Processing binary op: ", name
      );
      #endif
      auto out_val = fun(children[0]->template ProcessAs<emp::Datum>(),
                         children[1]->template ProcessAs<emp::Datum>());
      return GetSymbolTable().MakeTempSymbol(out_val);
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      children[0]->Write(os, offset);
      os << " " << name << " ";
      children[1]->Write(os, offset);
    }

    void PrintAST(std::ostream & os=std::cout, size_t indent=0) override {
      for (size_t i = 0; i < indent; ++i) os << " ";
      os << "ASTNode_Math2: " << GetName() << std::endl;
      for (auto child : children) child->PrintAST(os, indent+2);
    }
  };


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

      #ifndef NDEBUG
      emp::notify::Verbose(
        "Emplode::AST",
        "AST: Assigning: ", lhs->GetName(), " = ", rhs->GetName(), " (", rhs->AsString(), ")"
      );
      #endif

      bool success = lhs && lhs->CopyValue(*rhs);
      if (!success) {
        std::cerr << "Error: copy to '" << lhs->GetName() << "' failed" << std::endl;
        exit(1);
      }
      if (rhs->IsTemporary()) rhs.Delete();
      return lhs;
    }

    void PrintAST(std::ostream & os=std::cout, size_t indent=0) override {
      for (size_t i = 0; i < indent; ++i) os << " ";
      os << "ASTNode_Assign: " << GetName() << std::endl;
      for (auto child : children) child->PrintAST(os, indent+2);
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
      #ifndef NDEBUG
      emp::notify::Verbose(
        "Emplode::AST",
        "AST: Processing IF"
      );
      #endif

      double test = children[0]->ProcessAs<double>();             // Determine state of condition
      symbol_ptr_t out = nullptr;                                 // Prepare for output symbol.

      if (test != 0.0) out = children[1]->Process();              // Process if TRUE
      else if (children.size() > 2) out = children[2]->Process(); // Process if FALSE

      if (out && (out->IsBreak() || out->IsContinue())) return out; // Propagate break/continue
      if (out && out->IsTemporary()) out.Delete();                  // Clean up out, if needed
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

    void PrintAST(std::ostream & os=std::cout, size_t indent=0) override {
      for (size_t i = 0; i < indent; ++i) os << " ";
      os << "ASTNode_If: " << GetName() << std::endl;
      for (auto child : children) child->PrintAST(os, indent+2);
    }
  };

  class ASTNode_While : public ASTNode_Internal {
  public:
    ASTNode_While(node_ptr_t test, node_ptr_t body, int _line=-1) {
      AddChild(test);
      AddChild(body);
      line_id = _line;
    }

    symbol_ptr_t Process() override {
      #ifndef NDEBUG
      emp::notify::Verbose(
        "Emplode::AST",
        "AST: Processing WHILE"
      );
      #endif

      while (children[0]->ProcessAs<double>()) {
        symbol_ptr_t out = children[1]->Process();
        if (out) {
          if (out->IsBreak())     { break; }
          if (out->IsContinue())  { continue; }
          if (out->IsTemporary()) { out.Delete(); }
        }
      }

      return nullptr;
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      os << "WHILE (";
      children[0]->Write(os, offset);
      os << ") ";
      children[1]->Write(os, offset);
    }

    void PrintAST(std::ostream & os=std::cout, size_t indent=0) override {
      for (size_t i = 0; i < indent; ++i) os << " ";
      os << "ASTNode_While: " << GetName() << std::endl;
      for (auto child : children) child->PrintAST(os, indent+2);
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
      #ifndef NDEBUG
      emp::notify::Verbose(
        "Emplode::AST",
        "AST: Processing Call"
      );
      #endif


      symbol_ptr_t fun = children[0]->Process();

      // Collect all arguments and call
      symbol_vector_t args;
      for (size_t i = 1; i < children.size(); i++) {
        args.push_back(children[i]->Process());
      }

      emp::notify::Verbose(
        "Emplode::AST",
        "AST: Calling function '", fun->GetName(), " with ", args.size(), " arguments."
      );

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

    void PrintAST(std::ostream & os=std::cout, size_t indent=0) override {
      for (size_t i = 0; i < indent; ++i) os << " ";
      os << "ASTNode_Call: " << GetName() << std::endl;
      for (auto child : children) child->PrintAST(os, indent+2);
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

      #ifndef NDEBUG
      emp::notify::Verbose(
        "Emplode::AST",
        "AST: Processing Event"
      );
      #endif

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

    void PrintAST(std::ostream & os=std::cout, size_t indent=0) override {
      for (size_t i = 0; i < indent; ++i) os << " ";
      os << "ASTNode_Event: " << GetName() << std::endl;
      for (auto child : children) child->PrintAST(os, indent+2);
    }
  };

}

#endif
