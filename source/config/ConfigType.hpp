/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  ConfigType.hpp
 *  @brief Setup types for use in scripting.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_TYPE_H
#define MABE_CONFIG_TYPE_H

#include "emp/base/assert.hpp"

#include "ConfigEntry.hpp"
#include "ConfigScope.hpp"

namespace mabe {

  enum class BaseType {
    INVALID = 0, 
    VOID,
    VALUE,
    STRING,
    STRUCT
  };

  // Base class for types that we want to be used for scripting.
  class ConfigType {
  private:
    emp::Ptr<ConfigScope> cur_scope;
  
  public:
    // Some special, internal variables associated with each object.
    bool _active=true;       ///< Should this object be used in the current run?
    std::string _desc="";    ///< Special description for this object.
    
    // ---==  Configuration Management ==---

    /// Link a variable to a configuration entry - the value will default to the
    /// variabels crrent value, but be updated when configs are loaded.
    template <typename VAR_T>
    ConfigEntry_Linked<VAR_T> & LinkVar(VAR_T & var,
                                        const std::string & name,
                                        const std::string & desc) {
      return GetScope().LinkVar<VAR_T>(name, var, desc);
    }

    /// Link a configuration entry to a pair of functions - it automatically calls the set
    /// function when configs are loaded, and the get function when current value is needed.
    template <typename VAR_T>
    ConfigEntry_Functions<VAR_T> & LinkFuns(std::function<VAR_T()> get_fun,
                                            std::function<void(const VAR_T &)> set_fun,
                                            const std::string & name,
                                            const std::string & desc) {
      return GetScope().LinkFuns<VAR_T>(name, get_fun, set_fun, desc);
    }

  public:
    virtual void SetupScope(ConfigScope & scope) { cur_scope = &scope; }
    virtual void SetupConfig() = 0;
    virtual ~ConfigType() { }

    ConfigScope & GetScope() { emp_assert(!cur_scope.IsNull()); return *cur_scope; }
    const ConfigScope & GetScope() const { emp_assert(!cur_scope.IsNull()); return *cur_scope; }
  };
}

#endif
