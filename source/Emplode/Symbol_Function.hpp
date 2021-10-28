/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  Symbol_Function.hpp
 *  @brief Manages individual functions for config.
 *  @note Status: BETA
 */

#ifndef EMPLODE_SYMBOL_FUNCTION_HPP
#define EMPLODE_SYMBOL_FUNCTION_HPP

#include <string>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/tuple_utils.hpp"
#include "emp/meta/ValPack.hpp"

#include "Symbol.hpp"
#include "EmplodeTools.hpp"

namespace emplode {

  class Symbol_Function : public Symbol {
  private:
    using this_t = Symbol_Function;
    using symbol_ptr_t = emp::Ptr<Symbol>;
    using fun_t = std::function< symbol_ptr_t( const emp::vector<symbol_ptr_t> & ) >;
    fun_t fun;
    bool numeric_return = false;
    bool string_return = false;
    // size_t arg_count;

  public:
    template <typename FUN_T>
    Symbol_Function(const std::string & _name,
                    FUN_T _fun,
                    const std::string & _desc,
                    emp::Ptr<Symbol_Scope> _scope)
      : Symbol(_name, _desc, _scope), fun(EmplodeTools::WrapFunction(_name, _fun))
    {
      using return_t = typename emp::FunInfo<FUN_T>::return_t;
      numeric_return = std::is_scalar_v<return_t>;
      string_return = std::is_same<std::string, return_t>();
    }

    Symbol_Function(const Symbol_Function &) = default;
    emp::Ptr<Symbol> Clone() const override { return emp::NewPtr<this_t>(*this); }

    bool IsFunction() const override { return true; }
    bool HasNumericReturn() const override { return numeric_return; }
    bool HasStringReturn() const override { return string_return; }

    symbol_ptr_t Call( const emp::vector<symbol_ptr_t> & args ) override { return fun(args); }
  };

}

#endif
