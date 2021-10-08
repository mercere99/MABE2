/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigEntry_Scope.hpp
 *  @brief Manages a full scope with many config entries (or sub-scopes).
 *  @note Status: BETA
 * 
 *  DEVELOPER NOTES:
 *  - Need to fix Add() function to give a user-level error, rather than an assert on duplication.
 * 
 */

#ifndef MABE_CONFIG_SCOPE_H
#define MABE_CONFIG_SCOPE_H

#include "emp/base/map.hpp"

#include "ConfigEntry.hpp"
#include "ConfigEntry_Function.hpp"
#include "ConfigEntry_Linked.hpp"

namespace mabe {

  // Set of multiple config entries.
  class ConfigEntry_Scope : public ConfigEntry {
  protected:
    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    using const_entry_ptr_t = emp::Ptr<const ConfigEntry>;
    emp::map< std::string, entry_ptr_t > symbol_table;   ///< Map of names to entries.

    ///< If this scope represents a structure, identify the type (otherwise type is "")
    const std::string type;

    template <typename T, typename... ARGS>
    T & Add(const std::string & name, ARGS &&... args) {
      auto new_ptr = emp::NewPtr<T>(name, std::forward<ARGS>(args)...);
      emp_assert(!emp::Has(symbol_table, name), "Do not redeclare functions or variables!",
                 name);
      symbol_table[name] = new_ptr;
      return *new_ptr;
    }

    template <typename T, typename... ARGS>
    T & AddBuiltin(const std::string & name, ARGS &&... args) {
      T & result = Add<T, ARGS...>(name, std::forward<ARGS>(args)...);
      result.SetBuiltin();
      return result;
    }

  public:
    ConfigEntry_Scope(const std::string & _name,
                const std::string & _desc,
                emp::Ptr<ConfigEntry_Scope> _scope,
                const std::string & _type="")
      : ConfigEntry(_name, _desc, _scope), type(_type) { }

    ConfigEntry_Scope(const ConfigEntry_Scope & in) : ConfigEntry(in) {
      // Copy all defined variables/scopes/functions
      for (auto [name, ptr] : symbol_table) { symbol_table[name] = ptr->Clone(); }
    }
    ConfigEntry_Scope(ConfigEntry_Scope &&) = default;

    ~ConfigEntry_Scope() {
      // Clear up the symbol table.
      for (auto [name, ptr] : symbol_table) { ptr.Delete(); }
    }

    std::string GetTypename() const override { return type; }

    bool IsScope() const override { return true; }
    bool IsLocal() const override { return true; }  // @CAO, for now assuming all scopes are local!

    /// Set this entry to be a correctly-types scope pointer.
    emp::Ptr<ConfigEntry_Scope> AsScopePtr() override { return this; }

    /// Get an entry out of this scope; 
    entry_ptr_t GetEntry(std::string name) { return emp::Find(symbol_table, name, nullptr); }

    /// Lookup a variable, scanning outer scopes if needed
    entry_ptr_t LookupEntry(const std::string & name, bool scan_scopes=true) override {
      // See if this next entry is in the var list.
      auto it = symbol_table.find(name);

      // If this name is unknown, check with the parent scope!
      if (it == symbol_table.end()) {
        if (scope.IsNull() || !scan_scopes) return nullptr;  // No parent?  Just fail...
        return scope->LookupEntry(name);
      }

      // Otherwise we found it!
      return it->second;
    }

    /// Lookup a variable, scanning outer scopes if needed (in const context!)
    const_entry_ptr_t LookupEntry(const std::string & name, bool scan_scopes=true) const override {
      // See if this entry is in the var list.
      auto it = symbol_table.find(name);

      // If this name is unknown, check with the parent scope!
      if (it == symbol_table.end()) {
        if (scope.IsNull() || !scan_scopes) return nullptr;  // No parent?  Just fail...
        return scope->LookupEntry(name);
      }

      // Otherwise we found it!
      return it->second;
    }

    /// Add a configuration entry that is linked to a variable - the incoming variable sets
    /// the default and is automatically updated when configs are loaded.
    template <typename VAR_T>
    ConfigEntry_Linked<VAR_T> & LinkVar(const std::string & name,
                                        VAR_T & var,
                                        const std::string & desc,
                                        bool is_builtin = false) {
      if (is_builtin) return AddBuiltin<ConfigEntry_Linked<VAR_T>>(name, var, desc, this);
      return Add<ConfigEntry_Linked<VAR_T>>(name, var, desc, this);
    }

    /// Add a configuration entry that interacts through a pair of functions - the functions
    /// are automatically called any time the entry is accessed (get_fun) or changed (set_fun)
    template <typename VAR_T>
    ConfigEntry_LinkedFunctions<VAR_T> & LinkFuns(const std::string & name,
                                            std::function<VAR_T()> get_fun,
                                            std::function<void(const VAR_T &)> set_fun,
                                            const std::string & desc,
                                            bool is_builtin = false) {
      if (is_builtin) {
        return AddBuiltin<ConfigEntry_LinkedFunctions<VAR_T>>(name, get_fun, set_fun, desc, this);
      }
      return Add<ConfigEntry_LinkedFunctions<VAR_T>>(name, get_fun, set_fun, desc, this);
    }

    /// Add an internal variable of type String.
    ConfigEntry_StringVar & AddStringVar(const std::string & name, const std::string & desc) {
      return Add<ConfigEntry_StringVar>(name, "", desc, this);
    }

    /// Add an internal variable of type Value.
    ConfigEntry_DoubleVar & AddValueVar(const std::string & name, const std::string & desc) {
      return Add<ConfigEntry_DoubleVar>(name, 0.0, desc, this);
    }

    /// Add an internal scope inside of this one.
    ConfigEntry_Scope & AddScope(const std::string & name, const std::string & desc, const std::string & type="") {
      return Add<ConfigEntry_Scope>(name, desc, this, type);
    }

    /// Add a new user-defined function.
    template <typename RETURN_T, typename... ARGS>
    ConfigEntry_Function & AddFunction(const std::string & name,
                              std::function<RETURN_T(ARGS...)> fun,
                              const std::string & desc) {
      return Add<ConfigEntry_Function>(name, fun, desc, this);
    }

    /// Add a new function that is a standard part of the scripting language.
    template <typename RETURN_T, typename... ARGS>
    ConfigEntry_Function & AddBuiltinFunction(const std::string & name,
                                        std::function<RETURN_T(ARGS...)> fun,
                                        const std::string & desc) {
      return AddBuiltin<ConfigEntry_Function>(name, fun, desc, this);
    }

    /// Write out all of the parameters contained in this scope to the provided stream.
    const ConfigEntry & WriteContents(std::ostream & os=std::cout, const std::string & prefix="",
                                      size_t comment_offset=32) const {

      // Loop through all of the entires in this scope and Write them.
      for (auto [name, ptr] : symbol_table) {
        if (ptr->IsBuiltin()) continue; // Skip writing built-in entries.
        ptr->Write(os, prefix, comment_offset);
      }

      return *this;
    }

    /// Write out this scope AND it's contents to the provided stream.
    const ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="",
                              size_t comment_offset=32) const override
    {
      // If this is a built-in scope, don't print it.
      if (IsBuiltin()) return *this;

      // Declare this scope.
      std::string cur_line = prefix;
      if (IsLocal()) cur_line += emp::to_string(GetTypename(), " ");
      cur_line += name;

      bool has_body = emp::AnyOf(symbol_table, [](entry_ptr_t ptr){ return !ptr->IsBuiltin(); }; );
      
      // Only open this scope if there are contents.
      cur_line += has_body ? " { " : ";";
      os << cur_line;

      // Indent the comment for the description (if there is one)
      WriteDesc(os, comment_offset, cur_line.size());

      // If we have internal entries, write them out.
      if (has_body) {
        WriteContents(os, prefix+"  ", comment_offset);
        os << prefix << "}\n";      // Close the scope.
      }

      return *this;
    }

    /// Make a copy of this scope and all of the entries inside it.
    entry_ptr_t Clone() const override { return emp::NewPtr<ConfigEntry_Scope>(*this); }
  };

}
#endif
