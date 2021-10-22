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

  template <typename RETURN_T>
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


  template <typename FUN_T> struct WrapFunction_impl;

  // Specialization for functions with NO arguments
  template <typename RETURN_T>
  struct WrapFunction_impl<RETURN_T()> {

    template <typename FUN_T>
    static auto ConvertFun(FUN_T fun) {
      return [fun=fun]([[maybe_unused]] const entry_vector_t & args) {
        emp_assert(args.size() == 0, "Too many arguments (expected 0)", args.size());
        return ConvertReturn( fun() );
      };
    }

  };

  // Specialization for functions with AT LEAST ONE argument.
  template <typename RETURN_T, typename PARAM1_T, typename... PARAM_Ts>
  struct WrapFunction_impl<RETURN_T(PARAM1_T, PARAM_Ts...)> {
    using this_fun_t = RETURN_T(PARAM1_T, PARAM_Ts...);

    template <typename FUN_T>
    static auto ConvertFun(FUN_T fun) {        
      // If this function is already the correct type, just pass it along.
      if constexpr (std::is_same<this_fun_t, target_t>()) { return fun; }

      // Otherwise convert types as needed.
      else {
       return [fun=fun](const entry_vector_t & args) {
          emp_assert(args.size() == 1+sizeof...(PARAM_Ts), "Wrong argument count!");
          size_t i = 0;
          return ConvertReturn( fun(args[i++]->As<PARAM1_T>(), args[i++]->As<PARAM_Ts>()...) );
        };
      }
    }

    template <typename FUN_T>
    static auto ConvertMemberFun(const std::string & name, FUN_T fun) {  
      constexpr size_t NUM_PARAMS = sizeof...(PARAM_Ts);

      // If this function is already the correct type, just pass it along.
      if constexpr (NUM_PARAMS == 1) {
        using arg1_t = typename emp::FunInfo<FUN_T>::template arg_t<1>;
        if (std::is_reference_v<PARAM1_T> &&
            std::is_base_of_v<ConfigType, std::remove_reference_t<PARAM1_T>> &&
            std::is_same_v<arg1_t, const entry_vector_t &>) {
          return fun;
        }
      }

      // Otherwise convert types as needed.
      else {
       return [name=name,fun=fun](ConfigType & obj, const entry_vector_t & args) {
          emp_assert(args.size() == sizeof...(PARAM_Ts), "Wrong argument count!",
                     args.size(), sizeof...(PARAM_Ts));
          emp::Ptr<ConfigType> obj_ptr(&obj);
          auto typed_ptr = obj_ptr.DynamicCast<std::remove_reference_t<PARAM1_T>>();
          emp_assert(typed_ptr, "Internal error: member function call on wrong object type!");

          // Make sure we have the correct number of arguments.
          if (args.size() != NUM_PARAMS) {
            std::cerr << "Error in call to function '" << name
              << "'; expected " << NUM_PARAMS
              << " arguments, but received " << args.size() << "."
              << std::endl;
          }
          //@CAO should collect file position information for the above errors.

          size_t arg_id = 0;
          return ConvertReturn( fun(*typed_ptr, args[arg_id++]->As<PARAM_Ts>()...) );
        };
      }
    }

  };

  // Wrap a provided function to make it take a vector of Ptr<ConfigEntry> and return a
  // single Ptr<ConfigEntry> representing the result.
  template <typename FUN_T>
  static auto WrapFunction(FUN_T fun) {
    return WrapFunction_impl<typename emp::FunInfo<FUN_T>::fun_t>::ConvertFun(fun);
  }

  // Wrap a provided MEMBER function to make it take a reference to the object it is a member of
  // and a vector of Ptr<ConfigEntry> and return a single Ptr<ConfigEntry> representing the result.
  template <typename FUN_T>
  static auto WrapMemberFunction(emp::TypeID class_type, const std::string & name, FUN_T fun) {
    // Do some checks that will produce reasonable errors.
    using info_t = emp::FunInfo<FUN_T>;
    static_assert(info_t::num_args >= 1, "Member function add must always begin with an object reference.");
    using object_t = typename info_t::template arg_t<0>;
    using base_object_t = typename std::remove_cv_t< std::remove_reference_t<object_t> >;

    // Is the first parameter the correct type?
    static_assert(std::is_base_of<ConfigType, base_object_t>(),
                  "Member functions must take a reference to the associated ConfigType");
    emp_assert( class_type.IsType<base_object_t>(),
                "First parameter must match class type of member function being created!",
                emp::GetTypeID<object_t>(), class_type );

    return WrapFunction_impl<typename emp::FunInfo<FUN_T>::fun_t>::ConvertMemberFun(name, fun);
  }

}
}

#endif
