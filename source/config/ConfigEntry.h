/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigEntry.h
 *  @brief Manages a single configuration entry.
 *  @note Status: ALPHA
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
    std::string name;         ///< Unique name for this entry
    std::string desc;         ///< Description to put in comments for this entry.
    std::string default_val;  ///< String representing value to use in generated config file.
  
    using struct_t = emp::vector< emp::Ptr<ConfigEntry> >;
  public:
    ConfigEntry(const std::string & _name, const std::string & _desc="", const std::string & _dval="")
      : name(_name), desc(_desc), default_val(_dval) { }
    virtual ~ConfigEntry() { }

    virtual bool IsValue() const { return false; }
    virtual bool IsString() const { return false; }
    virtual bool IsStruct() const { return false; }

    virtual emp::Ptr<ConfigValue> AsValue() { return nullptr; }
    virtual emp::Ptr<ConfigString> AsString() { return nullptr; }
    virtual emp::Ptr<ConfigStruct> AsStruct() { return nullptr; }

    virtual ConfigType GetType() const { return BaseType::VOID; }

    virtual emp::Ptr<ConfigEntry> LookupEntry(std::string in_name) {
      return (in_name == "") ? this : nullptr;
    }
    virtual emp::Ptr<const ConfigEntry> LookupEntry(std::string in_name) const {
      return (in_name == "") ? this : nullptr;
    }
    virtual bool Has(std::string in_name) const { return (bool) LookupEntry(in_name); }

    virtual ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") = 0;
  };

  // Config entry that is a numerical value (double)
  class ConfigValue : public ConfigEntry {
  protected:
    double value;
  public:
    ConfigValue(const std::string & _name,
                const std::string & _desc="",
                const std::string & _dval="")
      : ConfigEntry(_name, _desc, _dval) { }
    ~ConfigValue() { }

    bool IsValue() const override { return true; }

    emp::Ptr<ConfigValue> AsValue() override{ return this; }

    ConfigType GetType() const override { return BaseType::VALUE; }

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
    ConfigString(const std::string & _name, const std::string & _desc="")
      : ConfigEntry(_name, _desc) { }
    ~ConfigString() { }

    bool IsString() const override { return true; }
    emp::Ptr<ConfigString> AsString() override{ return this; }

    ConfigType GetType() const override { return BaseType::STRING; }

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
    T & Add(const std::string & name, const std::string & desc) {
      auto new_ptr = emp::NewPtr<T>(name, desc);
      entries[name] = new_ptr;
      return *new_ptr;
    }
  public:
    ConfigStruct(const std::string & _name, const std::string & _desc="")
      : ConfigEntry(_name, _desc) { }
    ~ConfigStruct() { }

    bool IsStruct() const override { return true; }
    emp::Ptr<ConfigStruct> AsStruct() override{ return this; }

    ConfigType GetType() const override { return BaseType::STRUCT; }

    emp::Ptr<const ConfigEntry> LookupEntry(std::string in_name) const override {
      // If no name is provided, we must be at the correct entry.
      if (in_name == "") return this;

      // Pop off the scope name (or variable name if no scope)
      std::string next = emp::string_pop(in_name, '.');

      // See if this next entry is in the var list.
      auto it = entries.find(next);

      // If this name is unknown, fail!
      if (it == entries.end()) return nullptr;

      // Otherwise recursively call the entry (If no name is left, the next entry will return itself.)
      return it->second->LookupEntry(in_name);
    }

    emp::Ptr<ConfigEntry> LookupEntry(std::string in_name) override {
      // If no name is provided, we must be at the correct entry.
      if (in_name == "") return this;

      // Pop off the scope name (or variable name if no scope)
      std::string next = emp::string_pop(in_name, '.');

      // See if this next entry is in the var list.
      auto it = entries.find(next);

      // If this name is unknown, fail!
      if (it == entries.end()) return nullptr;

      // Otherwise recursively call the entry (If no name is left, the next entry will return itself.)
      return it->second->LookupEntry(in_name);
    }

    emp::Ptr<ConfigEntry> GetEntry(std::string in_name) {
      // Lookup this next entry is in the var list.
      auto it = entries.find(in_name);

      // If this name is unknown, fail!
      if (it == entries.end()) return nullptr;

      // Otherwise recursively call the entry (If no name is left, the next entry will return itself.)
      return it->second->LookupEntry(in_name);
    }

    auto & AddValue(const std::string & name, const std::string & desc, double value) {
      return Add<ConfigValue>(name, desc).Set(value);
    }
    auto & AddString(const std::string & name, const std::string & desc, const std::string & value) {
      return Add<ConfigString>(name, desc).Set(value);
    }
    auto & AddStruct(const std::string & name, const std::string & desc) {
      return Add<ConfigStruct>(name, desc);
    }

    ConfigEntry & Write(std::ostream & os=std::cout, const std::string & prefix="") override {
      if (desc.size()) os << prefix << "// " << desc << "\n";

      os << prefix << name << " = {\n";
      for (auto & x : entries) {
        x.second->Write(os, prefix+"  ");
      }
      os << predix << "}\n";
      return *this;
    }
  };

}
#endif
