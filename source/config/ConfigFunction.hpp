/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  ConfigFunction.hpp
 *  @brief Manages individual functions for config.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_FUNCTION_H
#define MABE_CONFIG_FUNCTION_H

#include <string>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/tuple_utils.hpp"
#include "emp/meta/ValPack.hpp"

#include "ConfigEntry.hpp"

namespace mabe {

  class ConfigFunction : public ConfigEntry {
  private:
    using this_t = ConfigFunction;
    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    using entry_vector_t = emp::vector<entry_ptr_t>;
    using fun_t = std::function< entry_ptr_t( const emp::vector<entry_ptr_t> & ) >;
    fun_t fun;
    bool numeric_return = false;
    bool string_return = false;
    // size_t arg_count;

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
    bool HasNumericReturn() const override { return numeric_return; }
    bool HasStringReturn() const override { return string_return; }

    /// Setup a function that takes NO arguments.
    template <typename RETURN_T>
    void SetFunction( std::function<RETURN_T()> in_fun ) {
      numeric_return = std::is_scalar_v<RETURN_T>;
      string_return = std::is_same<std::string, RETURN_T>();

      // Convert the function call to using entry pointers.
      fun = [in_fun, name=name, desc=desc](const emp::vector<entry_ptr_t> & args) -> emp::Ptr<ConfigEntry> {        
        // If arguments are passed in, we need to raise an error.
        if (args.size()) {
          return emp::NewPtr<ConfigEntry_Error>(
            "Function '", name, "' called with ", args.size(), " args, but ZERO expected."
          );
        }

        emp::Ptr<ConfigEntry> out_entry =
          emp::NewPtr<ConfigEntry_Var<RETURN_T>>("return value", in_fun(), desc, nullptr);
        out_entry->SetTemporary();
        return out_entry;
      };
    }

    /// Helper function to convert ASTs into the proper arguments.
    template <typename RETURN_T, typename... ARGS, auto... INDICES>
    void SetFunction_impl( std::function<RETURN_T(ARGS...)> in_fun, emp::ValPack<INDICES...> ) {
      fun = [in_fun, name=name, desc=desc](const entry_vector_t & args) -> emp::Ptr<ConfigEntry> {        
        // The call needs to have the correct number of arguments or else it throws an error.
        constexpr int NUM_ARGS = sizeof...(ARGS);
        if (args.size() != NUM_ARGS) {
          return emp::NewPtr<ConfigEntry_Error>(
            "Function '", name, "' called with ", args.size(), " args, but ", NUM_ARGS, " expected."
          );            
        }

        RETURN_T result = in_fun((args[INDICES]->template As< std::decay_t<ARGS> >())...);
        emp::Ptr<ConfigEntry> out_entry =
          emp::NewPtr<ConfigEntry_Var<RETURN_T>>("return value", result, desc, nullptr);
        out_entry->SetTemporary();
        return out_entry;
      };
    }

    /// Setup a function that takes AT LEAST ONE argument.
    template <typename RETURN_T, typename ARG1, typename... ARGS>
    void SetFunction( std::function<RETURN_T(ARG1, ARGS...)> in_fun ) {
      /// If we have only one argument and it is a `const emp::vector<emp::Ptr<ConfigEntry>> &`,
      /// assume that the function will handle any conversions itself.
      if constexpr (std::is_same<ARG1, const entry_vector_t &>() &&
                    sizeof...(ARGS) == 0) {
        fun = [in_fun, name=name, desc=desc](const entry_vector_t & args) -> emp::Ptr<ConfigEntry> {        
          RETURN_T result = in_fun(args);
          emp::Ptr<ConfigEntry> out_entry =
            emp::NewPtr<ConfigEntry_Var<RETURN_T>>("return value", result, desc, nullptr);
          out_entry->SetTemporary();
          return out_entry;
        };
      }

      /// Convert the function call to using entry pointers.
      else {
        SetFunction_impl( in_fun, emp::ValPackCount<sizeof...(ARGS)+1>() );
        // fun = [in_fun, name=name, desc=desc](const entry_vector_t & args) -> emp::Ptr<ConfigEntry> {        
        //   // The call needs to have the correct number of arguments or else it throws an error.
        //   constexpr int NUM_ARGS = sizeof...(ARGS) + 1;
        //   if (args.size() != NUM_ARGS) {
        //     return emp::NewPtr<ConfigEntry_Error>(
        //       "Function '", name, "' called with ", args.size(), " args, but ", NUM_ARGS, " expected."
        //     );            
        //   }

        //   size_t i = 1;
        //   RETURN_T result = in_fun(args[0]->As<ARG1>(), args[i++]->As<ARGS>()...);
        //   emp::Ptr<ConfigEntry> out_entry =
        //     emp::NewPtr<ConfigEntry_Var<RETURN_T>>("return value", result, desc, nullptr);
        //   out_entry->SetTemporary();
        //   return out_entry;
        // };
      }
    }

    entry_ptr_t Call( emp::vector<entry_ptr_t> args ) override { return fun(args); }

  };

}

#endif
