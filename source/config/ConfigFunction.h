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

  class ConfigFunction {
  private:
    std::string name;
    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    using fun_t = std::function< entry_ptr_t( const emp::vector<entry_ptr_t> & ) >;
    fun_t fun;
    size_t arg_count;

  public:
    ConfigFunction(const std::string & in_name) : name(in_name) { }

    template <typename RETURN_T, typename... ARGS>
    void AddFunction( std::function<RETURN_T(ARGS...)> in_fun ) override {
      // Convert the function call to using entry pointers.
      fun = [in_fun](const emp::vector<entry_ptr_t> & args) {        
        // The call needs to have the correct number of arguments or else it throws an error.
        constexpr int NUM_ARGS = sizeof...(ARGS);
        if (args.size() != NUM_ARGS) {
          std::stringstream msg;
          msg << "Function '" << name << "' called with " << args.size()
            << " args, but expected " << NUM_ARGS << ".";
          return emp::NewPtr<ConfigEntry_Error>(msg.str());
        }

        size_t i = 0;
        RETURN_T result = fun(args[i++].As<ARGS>()...);
        return emp::NewPtr<ConfigEntry_Var<RETURN_T>>(result)
      };
    }

    entry_ptr_t Call( emp::vector<entry_ptr_t> args ) { return fun(args); }

  };

}

#endif
