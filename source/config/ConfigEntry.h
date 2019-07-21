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
#include "tools/string_utils.h"

#include "ConfigType.h"

namespace mabe {

  class ConfigValue;
  class ConfigString;
  class ConfigStruct;

  class ConfigEntry {
  protected:
    std::string name;             ///< Unique name for this entry; empty name implied temporary.
    std::string desc;             ///< Description to put in comments for this entry.
    std::string default_val;      ///< String representing value to use in generated config file.
    emp::Ptr<ConfigStruct> scope; ///< Which scope was this variable defined in?
  
    using struct_t = emp::vector< emp::Ptr<ConfigEntry> >;
  public:
    ConfigEntry(const std::string & _name,
                const std::string & _desc,
                emp::Ptr<ConfigStruct> _scope)
      : name(_name), desc(_desc), scope(_scope) { }
    virtual ~ConfigEntry() { }

    virtual bool IsValue() const { return false; }
    virtual bool IsString() const { return false; }
    virtual bool IsStruct() const { return false; }

    // Test if this entry holds a temporary value.
    bool IsTemporary() const { return name == ""; }

    // Identify the structure that this variable was created in.
    emp::Ptr<ConfigStruct> GetScope() { return scope; }

    virtual emp::Ptr<ConfigValue> AsValue() { return nullptr; }
    virtual emp::Ptr<ConfigString> AsString() { return nullptr; }
    virtual emp::Ptr<ConfigStruct> AsStruct() { return nullptr; }

    /// Get the real type of this ConfigEntry
    virtual ConfigType GetType() const noexcept { return BaseType::VOID; }

    /// Set the default string for this entry.
    ConfigEntry & SetDefault(const std::string & in) { default_val = in; return *this; }

    /// Shift the current value to be the new default value.
    virtual void UpdateDefault() { default_val = ""; }

    virtual emp::Ptr<ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) {
      return (in_name == "") ? this : nullptr;
    }
    virtual emp::Ptr<const ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) const {
      return (in_name == "") ? this : nullptr;
    }
    virtual bool Has(std::string in_name) const { return (bool) LookupEntry(in_name); }

    virtual ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") = 0;
  };

  // Config entry that is a numerical value (double)
  class ConfigValue : public ConfigEntry {
  protected:
    double value = 0.0;
  public:
    ConfigValue(const std::string & _name,
                const std::string & _desc,
                emp::Ptr<ConfigStruct> _scope)
      : ConfigEntry(_name, _desc, _scope) { }
    ~ConfigValue() { }

    bool IsValue() const override { return true; }

    emp::Ptr<ConfigValue> AsValue() override{ return this; }

    ConfigType GetType() const noexcept override { return BaseType::VALUE; }
    void UpdateDefault() override { default_val = emp::to_string(value); }

    double Get() const { return value; }
    ConfigValue & Set(double in) { value = in; return *this; }

    ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") override {
      if (desc.size()) os << prefix << "// " << desc << "\n";
      os << prefix << name << " = ";

      // If a default value has been provided, print it.  Otherwise print the current value.
      if (default_val.size()) os << default_val;
      else os << value;
      os << ";\n";

      return *this;
    }
  };

  // Config entry that is a string.
  class ConfigString : public ConfigEntry {
  protected:
    std::string value;
  public:
    ConfigString(const std::string & _name,
                 const std::string & _desc,
                 emp::Ptr<ConfigStruct> _scope)
      : ConfigEntry(_name, _desc, _scope) { }
    ~ConfigString() { }

    bool IsString() const override { return true; }
    emp::Ptr<ConfigString> AsString() override{ return this; }

    ConfigType GetType() const noexcept override { return BaseType::STRING; }
    void UpdateDefault() override { default_val = value; }

    const std::string & Get() const { return value; }
    ConfigString & Set(const std::string & in) { value = in; return *this; }

    ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") override {
      if (desc.size()) os << prefix << "// " << desc << "\n";
      os << prefix << name << " = ";

      // If a default value has been provided, print it.  Otherwise print the current value.
      if (default_val.size()) os << default_val;
      else os << emp::to_literal(value);
      os << ";\n";

      return *this;
    }
  };

  // Set of multiple config entries.
  class ConfigStruct : public ConfigEntry {
  protected:
    emp::map< std::string, emp::Ptr<ConfigEntry> > entries;

    template <typename T>
    T & Add(const std::string & name, const std::string & desc, emp::Ptr<ConfigStruct> scope_ptr) {
      auto new_ptr = emp::NewPtr<T>(name, desc, scope_ptr);
      entries[name] = new_ptr;
      return *new_ptr;
    }
  public:
    ConfigStruct(const std::string & _name,
                 const std::string & _desc,
                 emp::Ptr<ConfigStruct> _scope)
      : ConfigEntry(_name, _desc, _scope) { }

    ~ConfigStruct() { }

    bool IsStruct() const override { return true; }
    emp::Ptr<ConfigStruct> AsStruct() override{ return this; }

    ConfigType GetType() const noexcept override { return BaseType::STRUCT; }
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

    auto & AddValue(const std::string & name, const std::string & desc, double value) {
      return Add<ConfigValue>(name, desc, this).Set(value);
    }
    auto & AddString(const std::string & name, const std::string & desc, const std::string & value) {
      return Add<ConfigString>(name, desc, this).Set(value);
    }
    auto & AddStruct(const std::string & name, const std::string & desc) {
      return Add<ConfigStruct>(name, desc, this);
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
  };

}
#endif
