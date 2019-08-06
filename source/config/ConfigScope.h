/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigScope.h
 *  @brief Manages a full scope with many conig entries (or sub-scopes).
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_SCOPE_H
#define MABE_CONFIG_SCOPE_H

#include "base/map.h"

#include "ConfigEntry.h"

namespace mabe {

  // Set of multiple config entries.
  class ConfigScope : public ConfigEntry {
  protected:
    emp::vector< emp::Ptr<ConfigEntry> > entry_list;           ///< Entries in order.
    emp::map< std::string, emp::Ptr<ConfigEntry> > entry_map;  ///< Entries with easy lookup.

    template <typename T, typename... ARGS>
    T & Add(const std::string & name, ARGS &&... args) {
      auto new_ptr = emp::NewPtr<T>(name, std::forward<ARGS>(args)...);
      entry_list.push_back(new_ptr);
      entry_map[name] = new_ptr;
      return *new_ptr;
    }
  public:
    ConfigScope(const std::string & _name,
                 const std::string & _desc,
                 emp::Ptr<ConfigScope> _scope)
      : ConfigEntry(_name, _desc, _scope) { }
    ConfigScope(const ConfigScope & in) : ConfigEntry(in) {
      for (const auto & x : in.entry_list) {
        auto new_ptr = x->Clone();
        entry_list.push_back(new_ptr);
        entry_map[x->GetName()] = new_ptr;
      }
    }
    ConfigScope(ConfigScope &&) = default;

    ~ConfigScope() {
      // Clear up all entries.
      for (auto & x : entry_list) { x.Delete(); }
    }

    bool IsScope() const override { return true; }

    /// Set this entry to be a correctly-types scope pointer.
    emp::Ptr<ConfigScope> AsScopePtr() override { return this; }

    /// Update the default value of all settings to be their current values.
    void UpdateDefault() override {
      // Recursively update all defaults within the structure.
      for (auto & x : entry_list) x->UpdateDefault();
      default_val = ""; /* @CAO: Need to spell out? */
    }

    /// Get an entry out of this scope; 
    emp::Ptr<ConfigEntry> GetEntry(std::string in_name) {
      // Lookup this next entry is in the var list.
      auto it = entry_map.find(in_name);

      // If this name is unknown, fail!
      if (it == entry_map.end()) return nullptr;

      // Otherwise return the entry.
      return it->second;
    }

    /// Lookup a variable, scanning outer scopes if needed
    emp::Ptr<ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) override {
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
    emp::Ptr<const ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) const override {
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
    template <typename VAR_T, typename DEFAULT_T>
    ConfigEntry_Linked<VAR_T> & LinkVar(VAR_T & var,
                                        const std::string & name,
                                        const std::string & desc,
                                        DEFAULT_T default_val) {
      return Add<ConfigEntry_Linked<VAR_T>>(name, var, desc, this);
    }

    /// Link a configuration entry to a pair of functions - it sets the new default and
    /// automatically calls the set function when configs are loaded.
    template <typename VAR_T, typename DEFAULT_T>
    ConfigEntry_Functions<VAR_T> & LinkFuns(std::function<VAR_T()> get_fun,
                                        std::function<void(const VAR_T &)> set_fun,
                                        const std::string & name,
                                        const std::string & desc,
                                        DEFAULT_T default_val) {
      return Add<ConfigEntry_Functions<VAR_T>>(name, get_fun, set_fun, desc, this);
    }

    /// Add a new scope inside of this one.
    ConfigScope & AddScope(const std::string & name, const std::string & desc) {
      return Add<ConfigScope>(name, desc, this);
    }

    /// Write out all of the parameters contained in this scope to the provided stream.
    ConfigEntry & WriteContents(std::ostream & os=std::cout, const std::string & prefix="",
                        size_t comment_offset=40) {

      // Loop through all of the entires in this scope and Write them.
      for (auto x : entry_list) {
        x->Write(os, prefix, comment_offset);
      }

      return *this;
    }

    /// Write out this scope AND it's contents to the provided stream.
    ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="",
                        size_t comment_offset=40) override {
      // Open this scope.
      os << prefix << name << " = { ";

      // Indent the comment for the description (if there is one)
      if (desc.size()) {
        size_t char_count = prefix.size() + name.size() + 5;
        while (char_count++ < comment_offset) os << " ";
        os << "// " << desc;
      }
      os << std::endl;

      WriteContents(os, prefix+"  ", comment_offset);

      // Close the scope.
      os << prefix << "}\n";
      return *this;
    }

    /// Make a copy of this scope and all of the entries inside it.
    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<ConfigScope>(*this); }
  };

}
#endif