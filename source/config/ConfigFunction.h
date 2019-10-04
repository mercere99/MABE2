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

  class ConfigFunctionBase {
  protected:
    std::string name;
    using entry_ptr_t = emp::Ptr<ConfigEntry>;

  public:
    ConfigFunctionBase(const std::string & _name) : name(_name) { ; }

    virtual entry_ptr_t Call( emp::vector<entry_ptr_t> args ) = 0;
  };

  template <typename RETURN, typename... ARGS>
  class ConfigFunction : public ConfigFunctionBase {
  private:
    using fun_t = std::function<RETURN(ARGS...)>;
    fun_t fun;

  public:
    ConfigFunction(fun_t _fun) : fun(_fun) { }

    entry_ptr_t Call( emp::vector<entry_ptr_t> args ) override {
      constexpr int NUM_ARGS = sizeof...(ARGS);

      // The call needs to have the correct number of arguments or else it throws an error.
      if (args.size() != NUM_ARGS) {
        std::stringstream msg;
        msg << "Function '" << name << "' called with " << args.size()
           << " args, but expected " << NUM_ARGS << ".";
        return emp::NewPtr<ConfigEntry_Error>(msg.str());
      }

      // Otherwise do the function call with the appropriate types!

      // Convert the vector into a tuple.
      size_t i = 0;
      auto result = fun(args[i++].As<ARGS>()...);
    }
  };

}

#endif
