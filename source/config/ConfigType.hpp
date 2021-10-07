/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigType.hpp
 *  @brief Setup types for use in scripting.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_TYPE_H
#define MABE_CONFIG_TYPE_H

#include "emp/base/assert.hpp"

#include "ConfigEntry.hpp"
#include "ConfigEntry_Scope.hpp"

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
    emp::Ptr<ConfigEntry_Scope> cur_scope;
  
  public:
    // Some special, internal variables associated with each object.
    bool _active=true;       ///< Should this object be used in the current run?
    std::string _desc="";    ///< Special description for this object.
    
    // ---==  Configuration Management ==---

    /// Link a variable to a configuration entry - the value will default to the
    /// variables current value, but be updated when configs are loaded.
    template <typename VAR_T>
    ConfigEntry_Linked<VAR_T> & LinkVar(VAR_T & var,
                                        const std::string & name,
                                        const std::string & desc,
                                        bool is_builtin = false) {
      return GetScope().LinkVar<VAR_T>(name, var, desc, is_builtin);
    }

    /// Link a configuration entry to a pair of functions - it automatically calls the set
    /// function when configs are loaded, and the get function when current value is needed.
    template <typename VAR_T>
    ConfigEntry_LinkedFunctions<VAR_T> & LinkFuns(std::function<VAR_T()> get_fun,
                                            std::function<void(const VAR_T &)> set_fun,
                                            const std::string & name,
                                            const std::string & desc,
                                            bool is_builtin = false) {
      return GetScope().LinkFuns<VAR_T>(name, get_fun, set_fun, desc, is_builtin);
    }

    // Helper functions and info.
    template <typename VAR_T>
    struct MenuEntry {
      VAR_T value;
      std::string name;
      std::string desc;

      MenuEntry(VAR_T v, const std::string & n, const std::string & d)
        : value(v), name(n), desc(d) {}
    };

    /// Link a set of menu option to a variable value.
    /// Each option should include three arguments:
    /// The return value, the option name, and the option description.
    template <typename VAR_T, typename... Ts>
    ConfigEntry_LinkedFunctions<std::string> & LinkMenu(VAR_T & var,
                                                  const std::string & name,
                                                  const std::string & desc,
                                                  const Ts &... entries) {
      auto menu = emp::BuildObjVector<MenuEntry<VAR_T>, 3>(entries...);

      // Build the "get" function: take the current value of the menu and return the name.
      std::function<std::string()> get_fun =
        [&var,menu](){
          for (const MenuEntry<VAR_T> & entry : menu) {
            if (var == entry.value) return entry.name;
          }
          return std::string("UNKNOWN");
        };

      // Build the "set" function: take the name of the menu option and update variable..
      std::function<void(std::string)> set_fun =
        [&var,name,menu](const std::string & entry_name){
          for (const MenuEntry<VAR_T> & entry : menu) {
            if (entry_name == entry.name) { var = entry.value; return; }
          };
          // AddError("Trying to set menu '", name, "' to '", entry_name, "'; does not exist.");
        };

      // Update the description to list all of the menu options.
      std::stringstream new_desc;

      // Start with the input description and add the description for each menu option.
      new_desc << desc;
      for (const MenuEntry<VAR_T> & entry : menu) {
        new_desc << "\n " << entry.name << ": " << entry.desc;
      }

      return GetScope().LinkFuns<std::string>(name, get_fun, set_fun, new_desc.str());
    }

  public:
    virtual void SetupScope(ConfigEntry_Scope & scope) { cur_scope = &scope; }
    virtual void SetupConfig() = 0;
    virtual ~ConfigType() { }

    ConfigEntry_Scope & GetScope() { emp_assert(!cur_scope.IsNull()); return *cur_scope; }
    const ConfigEntry_Scope & GetScope() const { emp_assert(!cur_scope.IsNull()); return *cur_scope; }
  };
}

#endif
