/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigScope.hpp
 *  @brief Manages a full scope with many conig entries (or sub-scopes).
 *  @note Status: ALPHA
 * 
 *  DEVELOPER NOTES:
 *  - Need to fix Add() function to give a user-level error, rather than an assert on duplication.
 * 
 */

#ifndef MABE_CONFIG_SCOPE_H
#define MABE_CONFIG_SCOPE_H

#include "emp/base/map.hpp"

#include "ConfigEntry.hpp"
#include "ConfigFunction.hpp"

namespace mabe {

  // Set of multiple config entries.
  class ConfigScope : public ConfigEntry {
  protected:
    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    emp::vector< entry_ptr_t > entry_list;            ///< Entries in order.
    emp::vector< entry_ptr_t > builtin_list;          ///< Built-in entries; not in config.
    emp::map< std::string, entry_ptr_t > entry_map;   ///< Entries with easy lookup.

    ///< If this scope represents a structure, identify the type (otherwise type is "")
    const std::string type;

    template <typename T, typename... ARGS>
    T & Add(const std::string & name, ARGS &&... args) {
      auto new_ptr = emp::NewPtr<T>(name, std::forward<ARGS>(args)...);
      entry_list.push_back(new_ptr);
      emp_assert(!emp::Has(entry_map, name), "Do not redeclare functions or variables!",
                 name);
      entry_map[name] = new_ptr;
      return *new_ptr;
    }

    template <typename T, typename... ARGS>
    T & AddBuiltin(const std::string & name, ARGS &&... args) {
      auto new_ptr = emp::NewPtr<T>(name, std::forward<ARGS>(args)...);
      builtin_list.push_back(new_ptr);
      emp_assert(!emp::Has(entry_map, name), "Do not redeclare built-in functions or variables!",
                 name);
      entry_map[name] = new_ptr;
      return *new_ptr;
    }
  public:
    ConfigScope(const std::string & _name,
                const std::string & _desc,
                emp::Ptr<ConfigScope> _scope,
                const std::string & _type="")
      : ConfigEntry(_name, _desc, _scope), type(_type) { }
    ConfigScope(const ConfigScope & in) : ConfigEntry(in) {
      // Copy all defined variables/scopes/functions
      for (const auto & x : in.entry_list) {
        auto new_ptr = x->Clone();
        entry_list.push_back(new_ptr);
        entry_map[x->GetName()] = new_ptr;
      }
      // Copy all built-in variables/scopes/functions
      for (const auto & x : in.builtin_list) {
        auto new_ptr = x->Clone();
        builtin_list.push_back(new_ptr);
        entry_map[x->GetName()] = new_ptr;
      }
    }
    ConfigScope(ConfigScope &&) = default;

    ~ConfigScope() {
      // Clear up all entries and built-ins.
      for (auto & x : entry_list) { x.Delete(); }
      for (auto & x : builtin_list) { x.Delete(); }
    }

    std::string GetTypename() const override { return type; }

    bool IsScope() const override { return true; }
    bool IsLocal() const override { return true; }  // @CAO, for now assuming all scopes are local!

    /// Set this entry to be a correctly-types scope pointer.
    emp::Ptr<ConfigScope> AsScopePtr() override { return this; }

    /// Get an entry out of this scope; 
    entry_ptr_t GetEntry(std::string in_name) {
      // Lookup this next entry is in the var list.
      auto it = entry_map.find(in_name);

      // If this name is unknown, fail!
      if (it == entry_map.end()) return nullptr;

      // Otherwise return the entry.
      return it->second;
    }

    /// Lookup a variable, scanning outer scopes if needed
    entry_ptr_t LookupEntry(const std::string & in_name, bool scan_scopes=true) override {
      // See if this next entry is in the var list.
      auto it = entry_map.find(in_name);

      // If this name is unknown, check with the parent scope!
      if (it == entry_map.end()) {
        if (scope.IsNull() || !scan_scopes) return nullptr;  // No parent?  Just fail...
        return scope->LookupEntry(in_name);
      }

      // Otherwise we found it!
      return it->second;
    }

    /// Lookup a variable, scanning outer scopes if needed (in constant context!)
    emp::Ptr<const ConfigEntry> LookupEntry(const std::string & in_name, bool scan_scopes=true) const override {
      // See if this entry is in the var list.
      auto it = entry_map.find(in_name);

      // If this name is unknown, check with the parent scope!
      if (it == entry_map.end()) {
        if (scope.IsNull() || !scan_scopes) return nullptr;  // No parent?  Just fail...
        return scope->LookupEntry(in_name);
      }

      // Otherwise we found it!
      return it->second;
    }

    /// Link a variable to a configuration entry - it sets the new default and
    /// automatically updates when configs are loaded.
    template <typename VAR_T>
    ConfigEntry_Linked<VAR_T> & LinkVar(const std::string & name,
                                        VAR_T & var,
                                        const std::string & desc,
                                        bool is_builtin = false) {
      if (is_builtin) return AddBuiltin<ConfigEntry_Linked<VAR_T>>(name, var, desc, this);
      return Add<ConfigEntry_Linked<VAR_T>>(name, var, desc, this);
    }

    /// Link a configuration entry to a pair of functions - it sets the new default and
    /// automatically calls the set function when configs are loaded.
    template <typename VAR_T>
    ConfigEntry_Functions<VAR_T> & LinkFuns(const std::string & name,
                                            std::function<VAR_T()> get_fun,
                                            std::function<void(const VAR_T &)> set_fun,
                                            const std::string & desc,
                                            bool is_builtin = false) {
      if (is_builtin) return AddBuiltin<ConfigEntry_Functions<VAR_T>>(name, get_fun, set_fun, desc, this);
      return Add<ConfigEntry_Functions<VAR_T>>(name, get_fun, set_fun, desc, this);
    }

    /// Add a new variable of type String.
    ConfigEntry_StringVar & AddStringVar(const std::string & name, const std::string & desc) {
      return Add<ConfigEntry_StringVar>(name, "", desc, this);
    }

    /// Add a new variable of type Value.
    ConfigEntry_DoubleVar & AddValueVar(const std::string & name, const std::string & desc) {
      return Add<ConfigEntry_DoubleVar>(name, 0.0, desc, this);
    }

    /// Add a new scope inside of this one.
    ConfigScope & AddScope(const std::string & name, const std::string & desc, const std::string & type="") {
      return Add<ConfigScope>(name, desc, this, type);
    }

    /// Add a new user-defined function.
    template <typename RETURN_T, typename... ARGS>
    ConfigFunction & AddFunction(const std::string & name,
                              std::function<RETURN_T(ARGS...)> fun,
                              const std::string & desc) {
      return Add<ConfigFunction>(name, fun, desc, this);
    }

    /// Add a new function that is a standard part of the scripting language.
    template <typename RETURN_T, typename... ARGS>
    ConfigFunction & AddBuiltinFunction(const std::string & name,
                                        std::function<RETURN_T(ARGS...)> fun,
                                        const std::string & desc) {
      return AddBuiltin<ConfigFunction>(name, fun, desc, this);
    }

    /// Write out all of the parameters contained in this scope to the provided stream.
    const ConfigEntry & WriteContents(std::ostream & os=std::cout, const std::string & prefix="",
                                      size_t comment_offset=32) const {

      // Loop through all of the entires in this scope and Write them.
      for (auto x : entry_list) {
        x->Write(os, prefix, comment_offset);
      }

      return *this;
    }

    /// Write out this scope AND it's contents to the provided stream.
    const ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="",
                              size_t comment_offset=32) const override
    {
      // If this is a built-in scope, don't print it.
      if (IsBuiltIn()) return *this;

      // Declare this scope.
      std::string cur_line = prefix;
      if (IsLocal()) cur_line += emp::to_string(GetTypename(), " ");
      cur_line += name;

      // Only open this scope if there are contents.
      cur_line += entry_list.size() ? " { " : ";";
      os << cur_line;

      // Indent the comment for the description (if there is one)
      WriteDesc(os, comment_offset, cur_line.size());

      // If we have internal entries, write them out.
      if (entry_list.size()) {
        WriteContents(os, prefix+"  ", comment_offset);
        os << prefix << "}\n";      // Close the scope.
      }

      return *this;
    }

    /// Make a copy of this scope and all of the entries inside it.
    entry_ptr_t Clone() const override { return emp::NewPtr<ConfigScope>(*this); }
  };

}
#endif
