/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigType.h
 *  @brief Setup types for use in scripting.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_TYPE_H
#define MABE_CONFIG_TYPE_H

#include "base/assert.h"

#include "ConfigEntry.h"
#include "ConfigScope.h"

namespace mabe {

  enum class BaseType {
    INVALID = 0, 
    VOID,
    VALUE,
    STRING,
    STRUCT
  };

  // Abstract base class for types that we want to be used for scripting..
  class ConfigType {
  private:
    emp::Ptr<ConfigScope> cur_scope;
  
  public:
    
    // ---==  Configuration Management ==---

    /// Link a variable to a configuration entry - it sets the new default and
    /// automatically updates when configs are loaded.
    template <typename VAR_T, typename DEFAULT_T>
    ConfigEntry_Linked<VAR_T> & LinkVar(VAR_T & var,
                                        const std::string & name,
                                        const std::string & desc,
                                        DEFAULT_T default_val) {
      return GetScope().LinkVar<VAR_T>(name, var, desc, default_val);
    }

    /// Link a configuration entry to a pair of functions - it sets the new default and
    /// automatically calls the set function when configs are loaded.
    template <typename VAR_T, typename DEFAULT_T>
    ConfigEntry_Functions<VAR_T> & LinkFuns(std::function<VAR_T()> get_fun,
                                            std::function<void(const VAR_T &)> set_fun,
                                            const std::string & name,
                                            const std::string & desc,
                                            DEFAULT_T default_val) {
      return GetScope().LinkFuns<VAR_T>(name, get_fun, set_fun, desc, default_val);
    }

  public:
    virtual void SetupScope(ConfigScope & scope) { cur_scope = &scope; }
    virtual void SetupConfig() = 0;
    virtual ~ConfigType() { }

    ConfigScope & GetScope() { return *cur_scope; }
    const ConfigScope & GetScope() const { return *cur_scope; }
  };
}

#endif