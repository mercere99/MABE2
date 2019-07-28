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
  
    double num_value = 0.0;       ///< Current numerical value of this config entry.
    std::string str_value = "";   ///< Current string value of this config entry.

    enum class Type { NONE=0, SCOPE,
                      BOOL, INT, UNSIGNED, DOUBLE,                                    // Values
                      STRING, FILENAME, PATH, URL, ALPHABETIC, ALPHANUMERIC, NUMERIC  // Strings
                    };
    type = Type::NONE;

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
    double GetValue() const { return num_value; }

    virtual bool IsNumeric() const { return false; }
    virtual bool IsBool() const { return false; }
    virtual bool IsInt() const { return false; }
    virtual bool IsDouble() const { return false; }
    virtual bool IsString() const { return false; }

    /// Set the default string for this entry.
    ConfigEntry & SetName(const std::string & in) { name = in; return *this; }
    ConfigEntry & SetDesc(const std::string & in) { desc = in; return *this; }
    ConfigEntry & SetDefault(const std::string & in) { default_val = in; return *this; }

    ConfigEntry & SetValue(double in) { value = in; return *this; }

    ConfigEntry & SetMin(double min) { range.SetLower(min); return *this; }
    ConfigEntry & SetMax(double max) { range.SetLower(max); return *this; }
    ConfigEntry & IntegerOnly(bool in=true) { interger_only - in; return *this; }

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

    /// Allocate a duplicate of this class.
    virtual emp::Ptr<ConfigEntry> Clone() const = 0;

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

  /// ConfigEntry can be linked directly to a real variable.
  template <typename T> class ConfigEntry_Linked {
  private:
    T & var;
  public:
    template <typename... ARGS>
    ConfigEntry_Linked(T & in_var, ARGS... && args)
    : ConfigEntry(std::forward<ARGS>(args)...), var(in_var) { ; }
  }


  // Set of multiple config entries.
  class ConfigScope : public ConfigEntry {
  protected:
    emp::map< std::string, emp::Ptr<ConfigEntry> > entries;

    template <typename T, typename... ARGS>
    T & Add(const std::string & name, ARGS... && args) {
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

    bool IsScope() const override { return true; }
    emp::Ptr<ConfigScope> AsScope() override{ return this; }

    ConfigType GetType() const noexcept override { return BaseType::STRUCT; }
    void UpdateDefault() override {
      // Recursively update all defaults within the structure.
      for (auto & x : entries) x.second->UpdateDefault();
      default_val = ""; /* @CAO: Need to spell out? */
    }

    virtual ConfigEntry & CopyValue(ConfigEntry & val) {
      emp_assert(val.IsScope());
      entries.clear();  // Erase anything currently in this struct.
      // Need to systematically duplicate all entires.
      ConfigScope & from = *(val.AsScope());
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

    template <typename T>
    ConfigValue_Link & LinkValue(T & var, const std::string & name, const std::string & desc, T default_val) {
      return Add<ConfigValue_Link>(name, desc, this, var);
    }

    ConfigString_Link & LinkString(std::string & var,
                                   const std::string & name,
                                   const std::string & desc,
                                   const std::string & default_val) {
      return Add<ConfigValue_String>(name, desc, this, var);
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
    auto & AddScope(const std::string & name, const std::string & desc) {
      return Add<ConfigScope>(name, desc, this);
    }

    ConfigScope & Replace(const std::string & name, emp::Ptr<ConfigEntry> entry) {
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

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<ConfigScope>(*this); }
  };

}
#endif
