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
#include "SymbolTableBase.hpp"

namespace emplode {

  class Symbol_Function : public Symbol {
  private:
    using this_t = Symbol_Function;
    using symbol_ptr_t = emp::Ptr<Symbol>;
    using fun_t = symbol_ptr_t( const emp::vector<symbol_ptr_t> & );
    using std_fun_t = std::function< fun_t >;

    std_fun_t fun;            // Unified-form function.
    emp::TypeID return_type;  // Native return type for original function.
    // size_t arg_count;

  public:
    Symbol_Function(const std::string & _name,
                    std_fun_t _fun,
                    const std::string & _desc,
                    emp::Ptr<Symbol_Scope> _scope,
                    emp::TypeID _ret_type)
      : Symbol(_name, _desc, _scope), fun(_fun), return_type(_ret_type)
    {
    }

    Symbol_Function(const Symbol_Function &) = default;
    emp::Ptr<Symbol> Clone() const override { return emp::NewPtr<this_t>(*this); }

    std::string GetTypename() const override { return "[Symbol_Function]"; }

    bool IsFunction() const override { return true; }
    bool HasNumericReturn() const override { return return_type.IsArithmetic(); }
    bool HasStringReturn() const override { return return_type.IsType<std::string>(); }

    /// Set this symbol to be a correctly-typed scope pointer.
    emp::Ptr<Symbol_Function> AsFunctionPtr() override { return this; }
    emp::Ptr<const Symbol_Function> AsFunctionPtr() const override { return this; }

    bool CopyValue(const Symbol & in) override {
      if (in.IsFunction() == false) {
          std::cerr << "Trying to assign `" << in.GetName() << "' to '" << GetName()
                    << "', but " << in.GetName() << " is not a Function." << std::endl;
        return false;   // Mis-matched types; failed to copy.
      }

      const Symbol_Function & in_fun = in.AsFunction();
      fun = in_fun.fun;
      return_type = in_fun.return_type;

      return true;
    }


    symbol_ptr_t Call( const emp::vector<symbol_ptr_t> & args ) override { return fun(args); }
  };

}

#endif
