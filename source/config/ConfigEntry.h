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
#include "base/Ptr.h"
#include "base/vector.h"
#include "meta/TypeID.h"
#include "tools/string_utils.h"

namespace mabe {

  class ConfigEntry {
  protected:
    std::string name;
    std::string desc;
  
  public:
    ConfigEntry(const std::string & _name, const std::string & _desc)
      : name(_name), desc(_desc) { }
    virtual ~ConfigEntry() { }
  };

  // Set of multiple config entries.
  class ConfigEntry_Struct : public ConfigEntry {
  protected:
    emp::vector< emp::Ptr<ConfigEntry> > entries;
  public:
    ConfigEntry_Struct(const std::string & _name, const std::string & _desc)
      : ConfigEntry(_name, _desc) { }
    ~ConfigEntry_Struct() { }
  };

  // Config entry that is a numerical value (double)
  class ConfigEntry_Value : public ConfigEntry {
  protected:
    double value;
  public:
    ConfigEntry_Value(const std::string & _name, const std::string & _desc)
      : ConfigEntry(_name, _desc) { }
    ~ConfigEntry_Value() { }
  };

}
#endif
