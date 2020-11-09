/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  ConfigAST.hpp
 *  @brief Manages Abstract Sytax Tree nodes for Config.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_AST_H
#define MABE_CONFIG_AST_H

#include "emp/base/assert.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"

#include "ConfigEntry.hpp"

namespace mabe {

  /// Base class for all AST Nodes.
  class ASTNode {
  protected:
    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    using entry_vector_t = emp::vector<entry_ptr_t>;

    using node_ptr_t = emp::Ptr<ASTNode>;
    using node_vector_t = emp::vector<node_ptr_t>;

    // Helper functions.
    emp::Ptr<ConfigEntry_DoubleVar> MakeTempDouble(double val) {
      auto out_ptr = emp::NewPtr<ConfigEntry_DoubleVar>("temp", val, "Temporary double", nullptr);
      out_ptr->SetTemporary();
      return out_ptr;    
    }

    emp::Ptr<ConfigEntry_StringVar> MakeTempString(const std::string & val) {
      auto out_ptr = emp::NewPtr<ConfigEntry_StringVar>("temp", val, "Temporary string", nullptr);
      out_ptr->SetTemporary();
      return out_ptr;    
    }
    size_t ast_tree_id = 0; 
  public:
    ASTNode() { ; }
    virtual ~ASTNode() { ; }

    virtual const std::string & GetName() const = 0;

    virtual bool IsLeaf() const { return false; }
    virtual bool IsInternal() const { return false; }

    virtual size_t GetNumChildren() const { return 0; }
    virtual node_ptr_t GetChild(size_t id) { emp_assert(false); return nullptr; }

    virtual entry_ptr_t Process() = 0;

    virtual void Write(std::ostream & os=std::cout, const std::string & offset="") const { }
    
    virtual void WriteASTFile(emp::File& file, size_t & cur_id){ }

    size_t GetASTTreeID() const { return ast_tree_id; }
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

    void AddChild(node_ptr_t child) { children.push_back(child); }
    
    void WriteASTFile(emp::File& file, size_t&  cur_id) override { 
      if(ast_tree_id == 0) ast_tree_id = cur_id++;
      file.Append(emp::to_string(ast_tree_id) + " [label=\"[Internal]\"]");
      for (auto child_ptr : children) {
        child_ptr->WriteASTFile(file, cur_id);
        file.Append(emp::to_string(ast_tree_id) + "->" + emp::to_string(child_ptr->GetASTTreeID()));
      }
    }
  };

  /// An ASTNode representing a leaf in the tree (i.e., a variable or literal)
  class ASTNode_Leaf : public ASTNode {
  protected:
    entry_ptr_t entry_ptr;   ///< Pointer to ConfigEntry at this leaf.
    bool own_entry;          ///< Should this node be in charge of deleting the entry pointer?

  public:
    ASTNode_Leaf(entry_ptr_t _ptr) : entry_ptr(_ptr), own_entry(_ptr->IsTemporary()) {
      entry_ptr->SetTemporary(false); // If this entry was temporary, it is now owned.
    }
    ~ASTNode_Leaf() { if (own_entry) entry_ptr.Delete(); }

    const std::string & GetName() const override { return entry_ptr->GetName(); }
    ConfigEntry & GetEntry() { return *entry_ptr; }

    bool IsLeaf() const override { return true; }

    entry_ptr_t Process() override { return entry_ptr; };

    void Write(std::ostream & os, const std::string &) const override {
      // If this is a variable, print the variable name,
      std::string output = entry_ptr->GetName();

      // If it is a literal, print the value.
      if (output == "") {
        output = entry_ptr->AsString();

        // If the entry is a string, convert it to a string literal.
        if (entry_ptr->IsString()) output = emp::to_literal(output);
      }
      os << output;
    }
    
    void WriteASTFile(emp::File& file, size_t & cur_id) override{
      // If this is a variable, print the variable name,
      std::string output = entry_ptr->GetName();

      // If it is a literal, print the value.
      if (output == "") {
        output = entry_ptr->AsString();

        // If the entry is a string, convert it to a string literal.
        if (entry_ptr->IsString()) output = "'" + output + "'";
      }
      if(ast_tree_id == 0) ast_tree_id = cur_id++;
      file.Append(emp::to_string(ast_tree_id) + " [label=\"[Leaf]\\n" + output + "\"]");
    }
  };

  class ASTNode_Block : public ASTNode_Internal {
  public:
    entry_ptr_t Process() override {
      for (auto node : children) {
        entry_ptr_t out = node->Process();
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
    
    void WriteASTFile(emp::File& file, size_t&  cur_id) override { 
      if(ast_tree_id == 0) ast_tree_id = cur_id++;
      file.Append(emp::to_string(ast_tree_id) + " [label=\"[Block]\"]");
      for (auto child_ptr : children) {
        child_ptr->WriteASTFile(file, cur_id);
        file.Append(emp::to_string(ast_tree_id) + "->" + emp::to_string(child_ptr->GetASTTreeID()));
      }
    }
  };

  /// Unary mathematical operations.
  class ASTNode_Math1 : public ASTNode_Internal {
  protected:
    // A unary operator take in a double and returns another one.
    std::function< double(double) > fun;
  public:
    ASTNode_Math1(const std::string & name) : ASTNode_Internal(name) { }

    void SetFun(std::function< double(double) > _fun) { fun = _fun; }

    entry_ptr_t Process() override {
      emp_assert(children.size() == 1);
      entry_ptr_t input_entry = children[0]->Process();     // Process child to get input entry
      double output_value = fun(input_entry->AsDouble());   // Run the function to get ouput value
      if (input_entry->IsTemporary()) input_entry.Delete(); // If we are done with input; delete!
      return MakeTempDouble(output_value);
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      os << name;
      children[0]->Write(os, offset);
    }
    void WriteASTFile(emp::File& file, size_t&  cur_id) override{
      if(ast_tree_id == 0) ast_tree_id = cur_id++;
      file.Append(emp::to_string(ast_tree_id) + " [label=\"[Math1]\\n" + name + "\"]");
      children[0]->WriteASTFile(file, cur_id);
      file.Append(emp::to_string(ast_tree_id) + "->" + emp::to_string(children[0]->GetASTTreeID()));
      children[1]->WriteASTFile(file, cur_id);
      file.Append(emp::to_string(ast_tree_id) + "->" + emp::to_string(children[1]->GetASTTreeID()));
    }
  };

  /// Binary mathematical operations.
  class ASTNode_Math2 : public ASTNode_Internal {
  protected:
    // A binary operator takes in two doubles and returns a third.
    std::function< double(double, double) > fun;
  public:
    ASTNode_Math2(const std::string & name) : ASTNode_Internal(name) { }

    void SetFun(std::function< double(double, double) > _fun) { fun = _fun; }

    entry_ptr_t Process() override {
      emp_assert(children.size() == 2);
      entry_ptr_t in1 = children[0]->Process();               // Process 1st child to input entry
      entry_ptr_t in2 = children[1]->Process();               // Process 2nd child to input entry
      double out_val = fun(in1->AsDouble(), in2->AsDouble()); // Run function; get ouput
      if (in1->IsTemporary()) in1.Delete();                   // If we are done with in1; delete!
      if (in2->IsTemporary()) in2.Delete();                   // If we are done with in2; delete!
      return MakeTempDouble(out_val);
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      children[0]->Write(os, offset);
      os << " " << name << " ";
      children[1]->Write(os, offset);
    }
    void WriteASTFile(emp::File& file, size_t&  cur_id) override{
      if(ast_tree_id == 0) ast_tree_id = cur_id++;
      file.Append(emp::to_string(ast_tree_id) + " [label=\"[Math2]\\n" + name + "\"]");
      children[0]->WriteASTFile(file, cur_id);
      file.Append(emp::to_string(ast_tree_id) + "->" + emp::to_string(children[0]->GetASTTreeID()));
      children[1]->WriteASTFile(file, cur_id);
      file.Append(emp::to_string(ast_tree_id) + "->" + emp::to_string(children[1]->GetASTTreeID()));
    }
  };

  class ASTNode_Assign : public ASTNode_Internal {
  public:
    ASTNode_Assign(node_ptr_t lhs, node_ptr_t rhs) {
      AddChild(lhs);
      AddChild(rhs);
    }

    entry_ptr_t Process() override {
      emp_assert(children.size() == 2);
      entry_ptr_t lhs = children[0]->Process();  // Determine the left-hand-side value.
      entry_ptr_t rhs = children[1]->Process();  // Determine the right-hand-side value.
      // @CAO Should make sure that lhs is properly assignable.
      lhs->CopyValue(*rhs);
      if (rhs->IsTemporary()) rhs.Delete();
      return lhs;
    }

    void Write(std::ostream & os, const std::string & offset) const override { 
      children[0]->Write(os, offset);
      os << " = ";
      children[1]->Write(os, offset);
    }
    void WriteASTFile(emp::File& file, size_t&  cur_id) override{
      if(ast_tree_id == 0) ast_tree_id = cur_id++;
      file.Append(emp::to_string(ast_tree_id) + " [label=\"=\"]");
      children[0]->WriteASTFile(file, cur_id);
      file.Append(emp::to_string(ast_tree_id) + "->" + emp::to_string(children[0]->GetASTTreeID()));
      children[1]->WriteASTFile(file, cur_id);
      file.Append(emp::to_string(ast_tree_id) + "->" + emp::to_string(children[1]->GetASTTreeID()));
    }
  };

  class ASTNode_Call : public ASTNode_Internal {
  public:
    ASTNode_Call(node_ptr_t fun, const node_vector_t & args) {
      AddChild(fun);
      for (auto arg : args) AddChild(arg);
    }

    entry_ptr_t Process() override {
      emp_assert(children.size() >= 1);
      entry_ptr_t fun = children[0]->Process();

      // Collect all arguments and call
      entry_vector_t args;
      for (size_t i = 1; i < children.size(); i++) {
        args.push_back(children[i]->Process());
      }
      entry_ptr_t result = fun->Call(args);

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
    void WriteASTFile(emp::File& file, size_t&  cur_id) override{
      if(ast_tree_id == 0) ast_tree_id = cur_id++;
      file.Append(emp::to_string(ast_tree_id) + " [label=\"[Call]\"]");
      for (auto child_ptr : children) {
        child_ptr->WriteASTFile(file, cur_id);
        file.Append(emp::to_string(ast_tree_id) + "->" + emp::to_string(child_ptr->GetASTTreeID()));
      }
    }
  };

  class ASTNode_Event : public ASTNode_Internal {
  protected:
    using setup_fun_t = std::function<void(node_ptr_t, const entry_vector_t &)>;
    setup_fun_t setup_event;

  public:
    ASTNode_Event(const std::string & event_name, node_ptr_t action, const node_vector_t & args, setup_fun_t in_fun)
     : ASTNode_Internal(event_name), setup_event(in_fun)
    {
      AddChild(action);
      for (auto arg : args) AddChild(arg);
    }

    entry_ptr_t Process() override {
      emp_assert(children.size() >= 1);
      entry_vector_t arg_entries;
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

    void WriteASTFile(emp::File& file, size_t&  cur_id) override{
      if(ast_tree_id == 0) ast_tree_id = cur_id++;
      file.Append(emp::to_string(ast_tree_id) + " [label=\"[Event]\\n" + name + "\"]");
      for (auto child_ptr : children) {
        child_ptr->WriteASTFile(file, cur_id);
        file.Append(emp::to_string(ast_tree_id) + "->" + emp::to_string(child_ptr->GetASTTreeID()));
      }
    }
  };

}

#endif
