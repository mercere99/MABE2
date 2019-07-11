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

namespace mabe {

  class ConfigEntry {
  protected:
    std::string name;
    std::string desc;
  
    using struct_t = emp::vector< emp::Ptr<ConfigEntry> >;
  public:
    ConfigEntry(const std::string & _name, const std::string & _desc)
      : name(_name), desc(_desc) { }
    virtual ~ConfigEntry() { }

    virtual bool IsValue() const { return false; }
    virtual bool IsString() const { return false; }
    virtual bool IsStruct() const { return false; }
  };

  // Config entry that is a numerical value (double)
  class ConfigEntry_Value : public ConfigEntry {
  protected:
    double value;
  public:
    ConfigEntry_Value(const std::string & _name, const std::string & _desc)
      : ConfigEntry(_name, _desc) { }
    ~ConfigEntry_Value() { }

    bool IsValue() const { return true; }

    ConfigEntry_Value & Set(double in) { value = in; return *this; }
  };

  // Config entry that is a numerical value (double)
  class ConfigEntry_String : public ConfigEntry {
  protected:
    std::string value;
  public:
    ConfigEntry_String(const std::string & _name, const std::string & _desc)
      : ConfigEntry(_name, _desc) { }
    ~ConfigEntry_String() { }

    bool IsString() const { return true; }

    ConfigEntry_Value & Set(const std::string & in) { value = in; return *this; }
  };

  // Set of multiple config entries.
  class ConfigEntry_Struct : public ConfigEntry {
  protected:
    emp::map< std::string, emp::Ptr<ConfigEntry> > entries;

    template <typename T>
    T & Add(const std::string & name, const std::string & desc) {
      auto new_ptr = emp::NewPtr<T>(name, desc);
      entries[name] = new_ptr;
      return *new_ptr;
    }
  public:
    ConfigEntry_Struct(const std::string & _name, const std::string & _desc)
      : ConfigEntry(_name, _desc) { }
    ~ConfigEntry_Struct() { }

    bool IsStruct() const { return true; }

    auto & AddValue(const std::string & name, const std::string & desc, double value) {
      return Add<ConfigEntry_Value>(name, desc).Set(value);
    }
    auto & AddString(const std::string & name, const std::string & desc, const std::string & value) {
      return Add<ConfigEntry_String>(name, desc).Set(value);
    }
    auto & AddStruct(const std::string & name, const std::string & desc) {
      return Add<ConfigEntry_Struct>(name, desc);
    }
  };

}
#endif
