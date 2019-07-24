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

  class ConfigPlaceholder;
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
    ConfigEntry(const ConfigEntry &) = default;
    virtual ~ConfigEntry() { }

    const std::string & GetName() const { return name; }
    const std::string & GetDesc() const { return desc; }
    const std::string & GetDefaultVal() const { return default_val; }
    emp::Ptr<ConfigStruct> GetScope() { return scope; }

    virtual bool IsPlaceholder() const { return false; }
    virtual bool IsValue() const { return false; }
    virtual bool IsString() const { return false; }
    virtual bool IsStruct() const { return false; }

    // Test if this entry holds a temporary value.
    // @CAO Could also do a nullptr scope?
    bool IsTemporary() const { return name == ""; }

    virtual emp::Ptr<ConfigPlaceholder> AsPlaceholder() { return nullptr; }
    virtual emp::Ptr<ConfigValue> AsValue() { return nullptr; }
    virtual emp::Ptr<ConfigString> AsString() { return nullptr; }
    virtual emp::Ptr<ConfigStruct> AsStruct() { return nullptr; }

    /// Get the real type of this ConfigEntry
    virtual ConfigType GetType() const noexcept { return BaseType::VOID; }

    /// Set the default string for this entry.
    ConfigEntry & SetName(const std::string & in) { name = in; return *this; }
    ConfigEntry & SetDesc(const std::string & in) { desc = in; return *this; }
    ConfigEntry & SetDefault(const std::string & in) { default_val = in; return *this; }

    /// Shift the current value to be the new default value.
    virtual void UpdateDefault() { default_val = ""; }

    /// Change the type of this variable to match another, if allowed.
    virtual bool UpdateType(ConfigEntry & other) { return false; }

    /// Change the value of this variable to match the one passed in, if possible.
    virtual ConfigEntry & CopyValue(ConfigEntry & val) = 0;

    virtual emp::Ptr<ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) {
      return (in_name == "") ? this : nullptr;
    }
    virtual emp::Ptr<const ConfigEntry> LookupEntry(std::string in_name, bool scan_scopes=true) const {
      return (in_name == "") ? this : nullptr;
    }
    virtual bool Has(std::string in_name) const { return (bool) LookupEntry(in_name); }

    /// Write out this entry as part of generating a configuration file.
    virtual ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") = 0;

    /// Allocate a duplicate of this class.
    virtual emp::Ptr<ConfigEntry> Clone() const = 0;
  };

  /// A specialized ConfigEntry where we don't have type details yet.
  class ConfigPlaceholder : public ConfigEntry {
  public:
    ConfigPlaceholder(const std::string & _name,
                      const std::string & _desc,
                      emp::Ptr<ConfigStruct> _scope)
      : ConfigEntry(_name, _desc, _scope) { }
    ConfigPlaceholder(const ConfigPlaceholder &) = default;
    ~ConfigPlaceholder() { }

    bool IsPlaceholder() const override { return true; }
    emp::Ptr<ConfigPlaceholder> AsPlaceholder() override { return this; }

    virtual ConfigEntry & CopyValue(ConfigEntry & val) {
      emp_error("Do not update placeholder values before setting type!");
      return *this;
    }

    ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") override {
      (void) os;  (void) prefix;
      emp_assert(false, "Temporary value being used for Write.");
      return *this;
    }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<ConfigPlaceholder>(*this); }
  };

  /// A ConfigEntry that is a numerical value (double)
  class ConfigValue : public ConfigEntry {
  protected:
    double value = 0.0;
  public:
    ConfigValue(const std::string & _name,
                const std::string & _desc,
                emp::Ptr<ConfigStruct> _scope)
      : ConfigEntry(_name, _desc, _scope) { }
    ConfigValue(const ConfigValue &) = default;
    ~ConfigValue() { }

    bool IsValue() const override { return true; }
    emp::Ptr<ConfigValue> AsValue() override { return this; }

    ConfigType GetType() const noexcept override { return BaseType::VALUE; }
    void UpdateDefault() override { default_val = emp::to_string(value); }

    double Get() const { return value; }
    ConfigValue & Set(double in) { value = in; return *this; }

    virtual ConfigEntry & CopyValue(ConfigEntry & val) {
      emp_assert(val.IsValue());
      value = val.AsValue()->Get();
      return *this;
    }

    ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") override {
      if (desc.size()) os << prefix << "// " << desc << "\n";
      os << prefix << name << " = ";

      // If a default value has been provided, print it.  Otherwise print the current value.
      if (default_val.size()) os << default_val;
      else os << value;
      os << ";\n";

      return *this;
    }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<ConfigValue>(*this); }
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
    ConfigString(const ConfigString &) = default;
    ~ConfigString() { }

    bool IsString() const override { return true; }
    emp::Ptr<ConfigString> AsString() override{ return this; }

    ConfigType GetType() const noexcept override { return BaseType::STRING; }
    void UpdateDefault() override { default_val = value; }

    const std::string & Get() const { return value; }
    ConfigString & Set(const std::string & in) { value = in; return *this; }

    virtual ConfigEntry & CopyValue(ConfigEntry & val) {
      emp_assert(val.IsString());
      value = val.AsString()->Get();
      return *this;
    }

    ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") override {
      if (desc.size()) os << prefix << "// " << desc << "\n";
      os << prefix << name << " = ";

      // If a default value has been provided, print it.  Otherwise print the current value.
      if (default_val.size()) os << default_val;
      else os << emp::to_literal(value);
      os << ";\n";

      return *this;
    }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<ConfigString>(*this); }
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
    ConfigStruct(const ConfigStruct & in) : ConfigEntry(in) {
      for (const auto & x : in.entries) {
        entries[x.first] = x.second->Clone();
      }
    }
    ConfigStruct(ConfigStruct &&) = default;

    ~ConfigStruct() {
      // Clear up all entries.
      for (auto & x : entries) { x.second.Delete(); }
    }

    bool IsStruct() const override { return true; }
    emp::Ptr<ConfigStruct> AsStruct() override{ return this; }

    ConfigType GetType() const noexcept override { return BaseType::STRUCT; }
    void UpdateDefault() override {
      // Recursively update all defaults within the structure.
      for (auto & x : entries) x.second->UpdateDefault();
      default_val = ""; /* @CAO: Need to spell out? */
    }

    virtual ConfigEntry & CopyValue(ConfigEntry & val) {
      emp_assert(val.IsStruct());
      entries.clear();  // Erase anything currently in this struct.
      // Need to systematically duplicate all entires.
      ConfigStruct & from = *(val.AsStruct());
      for (auto & x : from.entries) {
        entries[x.first] = x.second->Clone();
      }
      return *this;
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

    auto & AddPlaceholder(const std::string & name) {
      return Add<ConfigPlaceholder>(name,
                                    "Placeholder variable, to be replaced when we have more information.",
                                    this);
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

    ConfigStruct & Replace(const std::string & name, emp::Ptr<ConfigEntry> entry) {
      emp_assert(Has(name));   // Make sure the entry being replaced actually exists!
      entries[name].Delete();  // Delete the old entry.
      entries[name] = entry;   // Assigne the new entry.
      return *this;
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

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<ConfigStruct>(*this); }
  };

}
#endif
