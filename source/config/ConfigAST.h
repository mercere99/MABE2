/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigAST.h
 *  @brief Manages Abstract Sytax Tree nodes for Config.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_AST_H
#define MABE_CONFIG_AST_H

#include "base/assert.h"
#include "base/Ptr.h"
#include "base/vector.h"

#include "ConfigEntry.h"

namespace mabe {

  /// Base class for all AST Nodes.
  class ASTNode {
  protected:
    using entry_ptr_t = emp::Ptr<ConfigEntry>;

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
  public:
    ASTNode() { ; }
    virtual ~ASTNode() { ; }

    virtual const std::string & GetName() const = 0;

    virtual bool IsLeaf() const { return false; }
    virtual bool IsInternal() const { return false; }

    virtual size_t GetNumChildren() const { return 0; }
    virtual emp::Ptr<ASTNode> GetChild(size_t id) { emp_assert(false); return nullptr; }

    virtual entry_ptr_t Process() = 0;
  };

  /// An ASTNode representing an internal node.
  class ASTNode_Internal : public ASTNode {
  protected:
    std::string name;
    emp::vector< emp::Ptr<ASTNode> > children;

  public:
    ASTNode_Internal(const std::string & _name="") : name (_name) { }
    ~ASTNode_Internal() { 
      for (auto child : children) child.Delete();
    }

    const std::string & GetName() const override { return name; }

    bool IsInternal() const override { return true; }

    size_t GetNumChildren() const override { return children.size(); }
    emp::Ptr<ASTNode> GetChild(size_t id) override { return children[id]; }

    void AddChild(emp::Ptr<ASTNode> child) { children.push_back(child); }
  };

  /// An ASTNode representing a leaf in the tree (i.e., a variable or literal)
  class ASTNode_Leaf : public ASTNode {
  protected:
    entry_ptr_t entry_ptr;

  public:
    ASTNode_Leaf(entry_ptr_t _ptr) : entry_ptr(_ptr) { ; }
    ~ASTNode_Leaf() { if (entry_ptr->IsTemporary()) entry_ptr.Delete(); }

    const std::string & GetName() const override { return entry_ptr->GetName(); }
    ConfigEntry & GetEntry() { return *entry_ptr; }

    bool IsLeaf() const override { return true; }

    entry_ptr_t Process() override { return entry_ptr; };
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
  };

  /// Unary mathematical operations.
  class ASTNode_Math1 : public ASTNode_Internal {
  protected:
    // A unary operator take in a double and returns another one.
    std::function< double(double) > fun;
  public:
    void SetFun(std::function< double(double) > _fun) { fun = _fun; }

    entry_ptr_t Process() override {
      emp_assert(children.size() == 1);
      entry_ptr_t input_entry = children[0]->Process();     // Process child to get input entry
      double output_value = fun(input_entry->AsDouble());   // Run the function to get ouput value
      if (input_entry->IsTemporary()) input_entry.Delete(); // If we are done with input; delete!
      return MakeTempDouble(output_value);
    }
  };

  /// Binary mathematical operations.
  class ASTNode_Math2 : public ASTNode_Internal {
  protected:
    // A binary operator takes in two doubles and returns a third.
    std::function< double(double, double) > fun;
  public:
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
  };

  class ASTNode_Assign : public ASTNode_Internal {
  public:
    ASTNode_Assign(emp::Ptr<ASTNode> lhs, emp::Ptr<ASTNode> rhs) {
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
  };

  class ASTNode_Call : public ASTNode_Internal {
  public:
    entry_ptr_t Process() override {
      emp_assert(children.size() >= 1);
      entry_ptr_t fun = children[0]->Process();

      // Collect all arguments and call
      emp::vector<entry_ptr_t> args;
      for (size_t i = 1; i < children.size(); i++) {
        args.push_back(children[i]->Process());
      }
      entry_ptr_t result = fun->Call(args);

      // Cleanup and return
      for (auto arg : args) if (arg->IsTemporary()) arg.Delete();
      return result;
    }
  };

}

#endif