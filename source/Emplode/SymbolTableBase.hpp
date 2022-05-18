/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  SymbolTableBase.hpp
 *  @brief Tools for working with Symbol objects, especially for wrapping functions.
 *  @note Status: BETA
 */

#ifndef EMPLODE_SYMBOL_TABLE_BASE_HPP
#define EMPLODE_SYMBOL_TABLE_BASE_HPP

#include <string>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/tuple_utils.hpp"
#include "emp/debug/debug.hpp"
#include "emp/meta/FunInfo.hpp"
#include "emp/meta/ValPack.hpp"

#include "Symbol.hpp"

namespace emplode {

  // A base class for symbol table to provide low-level access to functions.
  class SymbolTableBase {
  public:
    virtual ~SymbolTableBase() { }

    using symbol_ptr_t = emp::Ptr<Symbol>;
    using symbol_vector_t = const emp::vector<symbol_ptr_t> &;
    using target_t = symbol_ptr_t( symbol_vector_t );

    /// Allocate a temporary (unnamed) object symbol with a given value.
    /// NOTE: Caller is responsible for deleting the created symbol!
    virtual emp::Ptr<Symbol_Object>
    MakeTempObjSymbol(emp::TypeID type_id, emp::Ptr<EmplodeType> value_ptr=nullptr) = 0;

    /// Allocate a temporary symbol with the given type and value.
    template <typename T>
    auto MakeTempSymbol(T value) {
      if constexpr (std::is_base_of<EmplodeType, T>()) {
        return MakeTempObjSymbol(emp::GetTypeID<T>(), &value);
      } else {
        auto out_symbol = emp::NewPtr<Symbol_Var>("__Temp", value, "", nullptr);
        out_symbol->SetTemporary();
        return out_symbol;
      }
    }

    /// Return a function that uses a C++ object's operator=() to copy a corresponding
    /// Emplode object.
    template <typename T>
    static auto DefaultCopyFun() {
      return [](const EmplodeType & from, EmplodeType & to) {
        emp::Ptr<const T> from_ptr = dynamic_cast<const T *>(&from);
        emp::Ptr<T> to_ptr = dynamic_cast<T *>(&to);
        if (!from_ptr || !to_ptr) return false;
        *to_ptr = *from_ptr;
        return true;
      };
    }

    /// Take in a value of an arbitrary type and convert it to a symbol (likely temporary)
    /// for use in performing a computation.
    template <typename T>
    symbol_ptr_t ValueToSymbol( T && value, const std::string & location ) {
      constexpr bool is_ref = std::is_lvalue_reference<T>();
      using base_t = std::remove_reference_t<T>;

      // If a return value is already a symbol pointer, just pass it through.
      if constexpr (std::is_same<T, symbol_ptr_t>()) {
        return value;
      }

      // If a return value is a basic type, wrap it in a temporary symbol
      else if constexpr (std::is_same<base_t, std::string>() ||
                         std::is_arithmetic<base_t>() ||
                         std::is_same<base_t, emp::Datum>() ||
                         std::is_same<base_t, emplode::Symbol_Var>()) {
        return MakeTempSymbol(value);
      }

      // If a value is a REFERENCE to an Emplode type, return its Symbol_Object.
      else if constexpr (is_ref && std::is_base_of<EmplodeType, base_t>()) {
        return value.AsScope().AsObject();
      }

      // If a return value is an Emplode type VALUE, build a temporary symbol for it.
      else if constexpr (!is_ref && std::is_base_of<EmplodeType, base_t>()) {
        return MakeTempSymbol(value);
      }

      // For now these are the only legal return type; raise error otherwise!
      else {
        std::cerr << "Failed to convert return type in " << location << std::endl;
        static_assert(emp::dependent_false<T>(),
                      "Invalid conversion of value to emplode::Symbol");
      }
    }


    /// A generic helper class for wrapping functions (must be specialized based on argument count)
    template <typename FUN_T, typename INDEX_Ts> struct WrapFunction_impl;

    /// Specialization for functions with NO arguments
    template <typename RETURN_T>
    struct WrapFunction_impl<RETURN_T(), emp::ValPack<>> {

      template <typename FUN_T>
      static constexpr int ParamCount(bool=false) { return 0; }

      template <typename FUN_T>
      static auto ConvertFun([[maybe_unused]] const std::string & name, FUN_T fun, SymbolTableBase & st) {
        return [name=name,fun=fun,&st]([[maybe_unused]] symbol_vector_t args) {
          emp_assert(args.size() == 0, "Too many arguments (expected 0)", name, args.size());
          return st.ValueToSymbol( fun(), name );
        };
      }

    };

    /// Specialization for functions with AT LEAST ONE argument.
    template <typename RETURN_T, typename PARAM1_T, typename... PARAM_Ts, auto... INDEX_VALS>
    struct WrapFunction_impl<RETURN_T(PARAM1_T, PARAM_Ts...), emp::ValPack<INDEX_VALS...>> {
      using this_fun_t = RETURN_T(PARAM1_T, PARAM_Ts...);
      static_assert( sizeof...(PARAM_Ts) == sizeof...(INDEX_VALS),
                    "Need one index for each parameter." );

      // How many parameters can this function have? -1 means any number!
      template <typename FUN_T>
      static constexpr int ParamCount(bool is_member=false) {
        using info_t = emp::FunInfo<FUN_T>;

        // If we have a member function, first arg is the object called from.
        if (is_member) {
          // How many parameters could the function have as a member function?
          if constexpr (sizeof...(PARAM_Ts) == 0) return 0;
          else if constexpr (sizeof...(PARAM_Ts) == 1) {
            using arg1_t = typename info_t::template arg_t<1>;
            if constexpr (std::is_same_v<arg1_t, symbol_vector_t>) return -1;             
          }
          else return sizeof...(PARAM_Ts);
        }        
        else {  // Not a member function; all arguments are fair game.
          if constexpr (sizeof...(PARAM_Ts) == 0) {
            if (std::is_same_v<PARAM1_T, symbol_vector_t>) return -1;
            return 1;
          }
          else return 1 + sizeof...(PARAM_Ts);
        }
      }


      // Stand-alone function (with at least one argument)...
      template <typename FUN_T>
      static auto ConvertFun(const std::string & name, FUN_T fun, SymbolTableBase & st) {        
        using info_t = emp::FunInfo<FUN_T>;
        static_assert(!std::is_same<typename info_t::return_t, void>(),
                      "Currently Emplode functions must provide a return value.");

        return [name=name,fun=fun,&st](symbol_vector_t args) {
          // If this function already takes a symbol_vector_t as its only parameter,
          // just pass it along.
          if constexpr (sizeof...(PARAM_Ts) == 0 &&
                        std::is_same_v<PARAM1_T, symbol_vector_t>) {
            return st.ValueToSymbol( fun(args), name );
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

            return st.ValueToSymbol(
              fun(args[0]->As<PARAM1_T>(), args[INDEX_VALS+1]->template As<PARAM_Ts>()...),
              name
            );
          }
        };      
      }

      // Member function (with at least one argument)...
      template <typename FUN_T>
      static auto ConvertMemberFun(const std::string & name, FUN_T fun, SymbolTableBase & st) {  
        using info_t = emp::FunInfo<FUN_T>;
        static_assert(!std::is_same<typename info_t::return_t, void>(),
                      "Currently Emplode functions must provide a return value.");

        static_assert(std::is_reference_v<PARAM1_T>,
                      "First parameter for supplied member functions must be reference to object");
        static_assert(std::is_base_of_v<EmplodeType, std::remove_reference_t<PARAM1_T>>,
                      "First parameter for supplied member functions must derived from EmplodeType");
        static_assert(info_t::num_args == sizeof...(PARAM_Ts) + 1,
                      "PARAM_Ts must match the extra arguments in member function.");

        return [name=name,fun=fun,&st](EmplodeType & obj, symbol_vector_t args) -> decltype(auto) {
          // Make sure the correct object type is used for first argument.
          emp::Ptr<EmplodeType> obj_ptr(&obj);
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

            return st.ValueToSymbol( fun(*typed_ptr), name );
          }

          // If this function already takes a symbol_vector_t as its only extra parameter,
          // just pass it along.
          else if constexpr (sizeof...(PARAM_Ts) == 1 &&
                        std::is_same_v<typename info_t::template arg_t<1>, symbol_vector_t>) {
            return st.ValueToSymbol( fun(*typed_ptr, args), name );
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

            return st.ValueToSymbol( fun(*typed_ptr, args[INDEX_VALS]->template As<PARAM_Ts>()...), name );
          }
        };
      }

    };

    // Wrap a provided function to make it take a vector of Ptr<Symbol> and return a
    // single Ptr<Symbol> representing the result.
    template <typename FUN_T>
    auto WrapFunction(const std::string & name, FUN_T fun) {
      using info_t = emp::FunInfo<FUN_T>;
      using fun_t = typename info_t::fun_t;
      if constexpr (info_t::num_args == 0) {
        return WrapFunction_impl<fun_t, emp::ValPack<>>::ConvertFun(name, fun, *this);
      } else {
        using index_t = emp::ValPackCount<info_t::num_args-1>;
        return WrapFunction_impl<fun_t, index_t>::ConvertFun(name, fun, *this);
      }
    }

    // Wrap a provided MEMBER function to make it take a reference to the object it is a member of
    // and a vector of Ptr<Symbol> and return a single Ptr<Symbol> representing the result.
    template <typename FUN_T>
    auto WrapMemberFunction([[maybe_unused]] emp::TypeID class_type,
                                  const std::string & name, FUN_T fun)
    {
      // Do some checks that will produce reasonable errors.
      using info_t = emp::FunInfo<FUN_T>;
      using index_t = emp::ValPackCount<info_t::num_args-1>;
      static_assert(info_t::num_args >= 1, "Member function add must always begin with an object reference.");

      // Is the first parameter the correct type?
      using object_t = typename info_t::template arg_t<0>;
      using base_object_t = typename std::remove_cv_t< std::remove_reference_t<object_t> >;
      static_assert(std::is_base_of<EmplodeType, base_object_t>(),
                    "Member functions must take a reference to the associated EmplodeType");
      emp_assert( class_type.IsType<base_object_t>(),
                  "First parameter must match class type of member function being created!",
                  emp::GetTypeID<object_t>(), class_type );

      using helper_t = WrapFunction_impl<typename emp::FunInfo<FUN_T>::fun_t, index_t>;
      return helper_t::ConvertMemberFun(name, fun, *this);
    }

  };

}

#endif
