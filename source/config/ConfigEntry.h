/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigEntry.h
 *  @brief Manages a single configuration entry.
 *  @note Status: ALPHA
 * 
 * 
 *  Development Notes:
 *  - When a ConfigEntry is used for a temporary value, it doesn't acutally need name, desc
 *    or default_val; we can probably remove these pretty easily to save on memory if needed.
 */

#ifndef MABE_CONFIG_ENTRY_H
#define MABE_CONFIG_ENTRY_H

#include "base/assert.h"
#include "base/map.h"
#include "base/Ptr.h"
#include "base/vector.h"
#include "meta/TypeID.h"
#include "tools/Range.h"
#include "tools/string_utils.h"

#include "ConfigType.h"

namespace mabe {

  class ConfigScope;

  class ConfigEntry {
  protected:
    std::string name;             ///< Unique name for this entry; empty name implied temporary.
    std::string desc;             ///< Description to put in comments for this entry.
    std::string default_val;      ///< String representing value to use in generated config file.
    emp::Ptr<ConfigScope> scope;  ///< Which scope was this variable defined in?
  
    enum class Type { NONE=0, SCOPE,
                      BOOL, INT, UNSIGNED, DOUBLE,                                    // Values
                      STRING, FILENAME, PATH, URL, ALPHABETIC, ALPHANUMERIC, NUMERIC  // Strings
                    };
    Type type = Type::NONE;

    // If we know the constraints on this parameter we can perform better error checking.
    emp::Range<double> range;  ///< Min and max values allowed for this config entry (if numerical).
    bool integer_only=false;   ///< Should we only allow integer values?

  public:
    ConfigEntry(const std::string & _name,
                const std::string & _desc,
                emp::Ptr<ConfigScope> _scope)
      : name(_name), desc(_desc), scope(_scope) { }
    ConfigEntry(const ConfigEntry &) = default;
    virtual ~ConfigEntry() { }

    const std::string & GetName() const { return name; }
    const std::string & GetDesc() const { return desc; }
    const std::string & GetDefaultVal() const { return default_val; }
    emp::Ptr<ConfigScope> GetScope() { return scope; }
    Type GetType() const noexcept { return type; }

    virtual bool IsNumeric() const { return false; }
    virtual bool IsBool() const { return false; }
    virtual bool IsInt() const { return false; }
    virtual bool IsDouble() const { return false; }
    virtual bool IsString() const { return false; }

    /// Set the default string for this entry.
    ConfigEntry & SetName(const std::string & in) { name = in; return *this; }
    ConfigEntry & SetDesc(const std::string & in) { desc = in; return *this; }
    ConfigEntry & SetDefault(const std::string & in) { default_val = in; return *this; }

    virtual double AsDouble() const = 0;
    virtual std::string AsString() const = 0;
    virtual ConfigEntry & SetValue(double in) = 0;
    virtual ConfigEntry & SetString(const std::string & in) = 0;

    virtual emp::Ptr<ConfigScope> AsScopePtr() { return nullptr; }
    ConfigScope & AsScope() {
      emp_assert(AsScopePtr());
      return *(AsScopePtr());
    }

    ConfigEntry & SetMin(double min) { range.SetLower(min); return *this; }
    ConfigEntry & SetMax(double max) { range.SetLower(max); return *this; }

    /// Shift the current value to be the new default value.
    virtual void UpdateDefault() { default_val = ""; }

    /// Change the value of this variable to match the one passed in, if possible.
    virtual ConfigEntry & CopyValue(ConfigEntry & val) = 0;

    virtual emp::Ptr<ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) {
      return (in_name == "") ? this : nullptr;
    }
    virtual emp::Ptr<const ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) const {
      return (in_name == "") ? this : nullptr;
    }
    virtual bool Has(std::string in_name) const { return (bool) LookupEntry(in_name); }

    /// Allocate a duplicate of this class.
    virtual emp::Ptr<ConfigEntry> Clone() const = 0;

    virtual ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") {
      if (desc.size()) os << prefix << "// " << desc << "\n";
      os << prefix << name << " = ";

      // If a default value has been provided, print it.  Otherwise print the current value.
      if (default_val.size()) os << default_val;
      else os << AsString();
      os << ";\n";

      return *this;
    }
  };

  /// ConfigEntry can be linked directly to a real variable.
  template <typename T>
  class ConfigEntry_Linked : public ConfigEntry {
  private:
    T & var;
  public:
    using this_t = ConfigEntry_Linked<T>;

    template <typename... ARGS>
    ConfigEntry_Linked(T & in_var, ARGS &&... args)
    : ConfigEntry(std::forward<ARGS>(args)...), var(in_var) { ; }
    ConfigEntry_Linked(const this_t &) = default;

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return (double) var; }
    std::string AsString() const override { return emp::to_string(var); }
    ConfigEntry & SetValue(double in) override { var = (T) in; return *this; }
    ConfigEntry & SetString(const std::string & in) override {
      var = emp::from_string<T>(in);
      return *this;
    }
  };

  /// Specializatin for ConfigEntry linked to a string variable.
  template <> class ConfigEntry_Linked<std::string> : public ConfigEntry {
  private:
    std::string & var;
  public:
    template <typename... ARGS>
    ConfigEntry_Linked(std::string & in_var, ARGS &&... args)
    : ConfigEntry(std::forward<ARGS>(args)...), var(in_var) { ; }

    double AsDouble() const override { return emp::from_string<double>(var); }
    std::string AsString() const override { return var; }
    ConfigEntry & SetValue(double in) override { var = emp::to_string(in); return *this; }
    ConfigEntry & SetString(const std::string & in) override { var = in; return *this; }
  };


  // Set of multiple config entries.
  class ConfigScope : public ConfigEntry {
  protected:
    emp::map< std::string, emp::Ptr<ConfigEntry> > entries;

    template <typename T, typename... ARGS>
    T & Add(const std::string & name, ARGS &&... args) {
      auto new_ptr = emp::NewPtr<T>(name, std::forward<ARGS>(args)...);
      entries[name] = new_ptr;
      return *new_ptr;
    }
  public:
    ConfigScope(const std::string & _name,
                 const std::string & _desc,
                 emp::Ptr<ConfigScope> _scope)
      : ConfigEntry(_name, _desc, _scope) { }
    ConfigScope(const ConfigScope & in) : ConfigEntry(in) {
      for (const auto & x : in.entries) {
        entries[x.first] = x.second->Clone();
      }
    }
    ConfigScope(ConfigScope &&) = default;

    ~ConfigScope() {
      // Clear up all entries.
      for (auto & x : entries) { x.second.Delete(); }
    }

    emp::Ptr<ConfigScope> AsScopePtr() override { return this; }

    void UpdateDefault() override {
      // Recursively update all defaults within the structure.
      for (auto & x : entries) x.second->UpdateDefault();
      default_val = ""; /* @CAO: Need to spell out? */
    }

    // Get an entry out of this scope; 
    emp::Ptr<ConfigEntry> GetEntry(std::string in_name) {
      // Lookup this next entry is in the var list.
      auto it = entries.find(in_name);

      // If this name is unknown, fail!
      if (it == entries.end()) return nullptr;

      // Otherwise return the entry.
      return it->second;
    }

    // Lookup a variable, scanning outer scopes if needed
    emp::Ptr<ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) override {
      // See if this next entry is in the var list.
      auto it = entries.find(in_name);

      // If this name is unknown, check with the parent scope!
      if (it == entries.end()) {
        if (scope.IsNull() || !scan_scopes) return nullptr;  // No parent?  Just fail...
        return scope->LookupEntry(in_name);
      }

      // Otherwise we found it!
      return it->second;
    }

    // Lookup a variable, scanning outer scopes if needed (in constant context!)
    emp::Ptr<const ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) const override {
      // See if this entry is in the var list.
      auto it = entries.find(in_name);

      // If this name is unknown, check with the parent scope!
      if (it == entries.end()) {
        if (scope.IsNull() || !scan_scopes) return nullptr;  // No parent?  Just fail...
        return scope->LookupEntry(in_name);
      }

      // Otherwise we found it!
      return it->second;
    }

    template <typename T>
    ConfigEntry_Linked<T> & LinkVar(T & var, const std::string & name, const std::string & desc, T default_val) {
      return Add<ConfigEntry_Linked<T>>(name, desc, this, var);
    }

    ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") override {
      if (desc.size()) os << prefix << "// " << desc << "\n";

      os << prefix << name << " = {\n";
      for (auto & x : entries) {
        x.second->Write(os, prefix+"  ");
      }
      os << prefix << "}\n";
      return *this;
    }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<ConfigScope>(*this); }
  };

}
#endif
