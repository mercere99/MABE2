/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  ConfigTools.hpp
 *  @brief Tools for working with ConfigEntry objects.
 *  @note Status: BETA
 */

#ifndef MABE_CONFIG_TOOLS_HPP
#define MABE_CONFIG_TOOLS_HPP

#include <string>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/tuple_utils.hpp"
#include "emp/meta/FunInfo.hpp"
#include "emp/meta/ValPack.hpp"

#include "ConfigEntry.hpp"
#include "ConfigTools.hpp"

namespace mabe {
namespace ConfigTools {

  using entry_ptr_t = emp::Ptr<ConfigEntry>;
  using entry_vector_t = emp::vector<entry_ptr_t>;
  using target_t = entry_ptr_t( const entry_vector_t & );

  // Use ConfigTools::MakeTempEntry(value) to quickly allocate a temporary entry with a
  // given value.  NOTE: Caller is responsible for deleting the created entry!
  template <typename VALUE_T>
  static emp::Ptr<ConfigEntry_Var<VALUE_T>> MakeTempEntry(VALUE_T value) {
    auto out_entry = emp::NewPtr<ConfigEntry_Var<VALUE_T>>("__Temp", value, "", nullptr);
    out_entry->SetTemporary();
    return out_entry;
  }


  template <typename FUN_T> struct WrapFunction_impl;

  template <typename RETURN_T, typename... PARAM_Ts>
  struct WrapFunction_impl<RETURN_T(PARAM_Ts...)> {

    static auto ConvertReturn( RETURN_T && return_value ) {
      // If a return value is already an entry pointer, just pass it through.
      if constexpr (std::is_same<RETURN_T, entry_ptr_t>()) {
        return return_value;
      }

      // If a return value is a basic type, wrap it in a temporary entry
      if constexpr (std::is_same<RETURN_T, std::string>() ||
                    std::is_arithmetic<RETURN_T>()) {
        return MakeTempEntry(return_value);
      }

      // For now these are the only legal return type; raise error otherwise!
      else {
        emp::ShowType<RETURN_T>{};
        static_assert(emp::dependent_false<RETURN_T>(),
                      "Invalid return value in ConfigEntry_Function::SetFunction()");
      }
    }

    template <typename FUN_T>
    static auto ConvertFun(FUN_T fun) {        
      // If this function is already the correct type, just pass it along.
      if constexpr (std::is_same<RETURN_T(PARAM_Ts...), target_t>()) { return fun; }

      // Otherwise convert types as needed.
      else {
       return [fun=fun](const entry_vector_t & args) {
          emp_assert(args.size() == sizeof...(PARAM_Ts), "Wrong argument count!");
          size_t i = 0;
          return ConvertReturn( fun(args[i]->As<PARAM_Ts>()...) );
        };
      }

    }
  };

  // Wrap a provided function to make sure it takes a vector of Ptr<ConfigEntry> and returns a
  // single Ptr<ConfigEntry> representing the result.
  template <typename FUN_T>
  static auto WrapFunction(FUN_T fun) {
    return WrapFunction_impl<typename emp::FunInfo<FUN_T>::fun_t>::ConvertFun(fun);
  }

}
}

#endif
