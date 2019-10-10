/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigFunction.h
 *  @brief Manages functions for config..
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_FUNCTION_H
#define MABE_CONFIG_FUNCTION_H

#include <string>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/tuple_utils.h"

#include "ConfigEntry.h"

namespace mabe {

  class ConfigFunction : public ConfigEntry {
  private:
    using this_t = ConfigFunction;
    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    using fun_t = std::function< entry_ptr_t( const emp::vector<entry_ptr_t> & ) >;
    fun_t fun;
    size_t arg_count;

  public:
    ConfigFunction(const std::string & _name,
                   const std::string & _desc,
                   emp::Ptr<ConfigScope> _scope)
      : ConfigEntry(_name, _desc, _scope) { ; }

    template <typename RETURN_T, typename... ARGS>
    ConfigFunction(const std::string & _name,
                   std::function<RETURN_T(ARGS...)> _fun,
                   const std::string & _desc,
                   emp::Ptr<ConfigScope> _scope)
      : ConfigEntry(_name, _desc, _scope) { SetFunction(_fun); }

    ConfigFunction(const ConfigFunction &) = default;

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    bool IsFunction() const override { return true; }

    template <typename RETURN_T, typename... ARGS>
    void SetFunction( std::function<RETURN_T(ARGS...)> in_fun ) {
      // Convert the function call to using entry pointers.
      fun = [this, in_fun](const emp::vector<entry_ptr_t> & args) -> emp::Ptr<ConfigEntry> {        
        // The call needs to have the correct number of arguments or else it throws an error.
        constexpr int NUM_ARGS = sizeof...(ARGS);
        if (args.size() != NUM_ARGS) {
          std::stringstream msg;
          msg << "Function '" << name << "' called with " << args.size()
            << " args, but expected " << NUM_ARGS << ".";
          return emp::NewPtr<ConfigEntry_Error>(msg.str());
        }

        size_t i = 0;
        RETURN_T result = in_fun(args[i++]->As<ARGS>()...);
        emp::Ptr<ConfigEntry> out_entry =
          emp::NewPtr<ConfigEntry_Var<RETURN_T>>("return value", result, desc, nullptr);
        out_entry->SetTemporary();
        return out_entry;
      };
    }

    entry_ptr_t Call( emp::vector<entry_ptr_t> args ) { return fun(args); }

  };

}

#endif
