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
    else if constexpr (std::is_same<RETURN_T, std::string>() ||
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


  template <typename FUN_T, typename INDEX_Ts> struct WrapFunction_impl;

  // Specialization for functions with NO arguments
  template <typename RETURN_T>
  struct WrapFunction_impl<RETURN_T(), emp::ValPack<>> {

    template <typename FUN_T>
    static auto ConvertFun([[maybe_unused]] const std::string & name, FUN_T fun) {
      return [name=name,fun=fun]([[maybe_unused]] const entry_vector_t & args) {
        emp_assert(args.size() == 0, "Too many arguments (expected 0)", name, args.size());
        return ConvertReturn( fun() );
      };
    }

  };

  // Specialization for functions with AT LEAST ONE argument.
  template <typename RETURN_T, typename PARAM1_T, typename... PARAM_Ts, auto... INDEX_VALS>
  struct WrapFunction_impl<RETURN_T(PARAM1_T, PARAM_Ts...), emp::ValPack<INDEX_VALS...>> {
    using this_fun_t = RETURN_T(PARAM1_T, PARAM_Ts...);
    static_assert( sizeof...(PARAM_Ts) == sizeof...(INDEX_VALS),
                   "Need one index for each parameter." );

    template <typename FUN_T>
    static auto ConvertFun(const std::string & name, FUN_T fun) {        
      return [name=name,fun=fun](const entry_vector_t & args) {
        // If this function already takes a const entry_vector_t & as its only parameter,
        // just pass it along.
        if constexpr (sizeof...(PARAM_Ts) == 0 &&
                      std::is_same_v<PARAM1_T, const entry_vector_t &>) {
          return ConvertReturn( fun(args) );
        }

        // Otherwise make sure we have the correct arguments.
        else {
          constexpr size_t NUM_PARAMS = 1 + sizeof...(PARAM_Ts);
          if (args.size() != NUM_PARAMS) {
            std::cerr << "Error in call to function '" << name
              << "'; expected " << NUM_PARAMS
              << " arguments, but received " << args.size() << "."
              << std::endl;
          }
          //@CAO should collect file position information for the above errors.

          return ConvertReturn( fun(args[0]->As<PARAM1_T>(),
                                args[INDEX_VALS]->template As<PARAM_Ts>()...) );
        }
      };      
    }

    template <typename FUN_T>
    static auto ConvertMemberFun(const std::string & name, FUN_T fun) {  
      using info_t = emp::FunInfo<FUN_T>;

      static_assert(std::is_reference_v<PARAM1_T>,
                    "First parameter for supplied member functions must be reference to object");
      static_assert(std::is_base_of_v<ConfigType, std::remove_reference_t<PARAM1_T>>,
                    "First parameter for supplied member functions must derived from ConfigType");
      static_assert(info_t::num_args == sizeof...(PARAM_Ts) + 1,
                    "PARAM_Ts must match the extra arguments in member function.");

      return [name=name,fun=fun](ConfigType & obj, const entry_vector_t & args) {
        // Make sure the correct object type is used for first argument.
        emp::Ptr<ConfigType> obj_ptr(&obj);
        auto typed_ptr = obj_ptr.DynamicCast<std::remove_reference_t<PARAM1_T>>();
        emp_assert(typed_ptr, "Internal error: member function call on wrong object type!", name);

        // If this member function takes no additional arguments just call it without any!
        if constexpr (sizeof...(PARAM_Ts) == 0) {
          if (args.size() != 0) {
            std::cerr << "Error in call to function '" << name
              << "'; expected ZERO arguments, but received " << args.size() << "."
              << std::endl;
          }
          //@CAO should collect file position information for the above errors.

          return ConvertReturn( fun(*typed_ptr) );
        }

        // If this function already takes a const entry_vector_t & as its only extra parameter,
        // just pass it along.
        else if constexpr (sizeof...(PARAM_Ts) == 1 &&
                      std::is_same_v<typename info_t::template arg_t<1>, const entry_vector_t &>) {
          return ConvertReturn( fun(*typed_ptr, args) );
        }

        // Otherwise make sure we have the correct arguments.
        else {
          constexpr size_t NUM_PARAMS = sizeof...(PARAM_Ts);
          if (args.size() != NUM_PARAMS) {
            std::cerr << "Error in call to function '" << name
              << "'; expected " << NUM_PARAMS
              << " arguments, but received " << args.size() << "."
              << std::endl;
          }
          //@CAO should collect file position information for the above errors.

          return ConvertReturn( fun(*typed_ptr, args[INDEX_VALS]->template As<PARAM_Ts>()...) );
        }
      };
    }

  };

  // Wrap a provided function to make it take a vector of Ptr<ConfigEntry> and return a
  // single Ptr<ConfigEntry> representing the result.
  template <typename FUN_T>
  static auto WrapFunction(const std::string & name, FUN_T fun) {
    using info_t = emp::FunInfo<FUN_T>;
    using fun_t = typename info_t::fun_t;
    if constexpr (info_t::num_args == 0) {
      return WrapFunction_impl<fun_t, emp::ValPack<>>::ConvertFun(name, fun);
    } else {
      using index_t = emp::ValPackCount<info_t::num_args-1>;
      return WrapFunction_impl<fun_t, index_t>::ConvertFun(name, fun);
    }
  }

  // Wrap a provided MEMBER function to make it take a reference to the object it is a member of
  // and a vector of Ptr<ConfigEntry> and return a single Ptr<ConfigEntry> representing the result.
  template <typename FUN_T>
  static auto WrapMemberFunction(emp::TypeID class_type, const std::string & name, FUN_T fun) {
    // Do some checks that will produce reasonable errors.
    using info_t = emp::FunInfo<FUN_T>;
    using index_t = emp::ValPackCount<info_t::num_args-1>;
    static_assert(info_t::num_args >= 1, "Member function add must always begin with an object reference.");

    // Is the first parameter the correct type?
    using object_t = typename info_t::template arg_t<0>;
    using base_object_t = typename std::remove_cv_t< std::remove_reference_t<object_t> >;
    static_assert(std::is_base_of<ConfigType, base_object_t>(),
                  "Member functions must take a reference to the associated ConfigType");
    emp_assert( class_type.IsType<base_object_t>(),
                "First parameter must match class type of member function being created!",
                emp::GetTypeID<object_t>(), class_type );

    return WrapFunction_impl<typename emp::FunInfo<FUN_T>::fun_t, index_t>::ConvertMemberFun(name, fun);
  }

}
}

#endif
