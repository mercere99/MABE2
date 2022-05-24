/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  Symbol_Linked.hpp
 *  @brief Manages a configuration entry linked to another variable or functions.
 *  @note Status: BETA
 */

#ifndef EMPLODE_SYMBOL_LINKED_HPP
#define EMPLODE_SYMBOL_LINKED_HPP

#include <type_traits>

#include "Symbol.hpp"

namespace emplode {

  /// Symbol can be linked directly to a real variable.
  template <typename T>
  class Symbol_Linked : public Symbol {
  private:
    T & var;
  public:
    using this_t = Symbol_Linked<T>;

    template <typename... ARGS>
    Symbol_Linked(const std::string & in_name, T & in_var, ARGS &&... args)
      : Symbol(in_name, std::forward<ARGS>(args)...), var(in_var) { ; }
    Symbol_Linked(const this_t &) = default;

    std::string GetTypename() const override {
      if constexpr (std::is_scalar_v<T>) return "[LinkedValue]";
      else return "[Error:InvalidLinkedType]";
    }

    emp::Ptr<Symbol> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return (double) var; }
    std::string AsString() const override { return emp::to_string(var); }
    Symbol & SetValue(double in) override { var = (T) in; return *this; }
    Symbol & SetString(const std::string & in) override {
      var = emp::from_string<T>(in);
      return *this;
    }

    bool IsNumeric() const override { return std::is_scalar_v<T>; }

    bool CopyValue(const Symbol & in) override { var = in.AsDouble(); return true; }
  };

  /// Specialization for Symbol linked to a string variable.
  template <>
  class Symbol_Linked<std::string> : public Symbol {
  private:
    std::string & var;
  public:
    using this_t = Symbol_Linked<std::string>;

    template <typename... ARGS>
    Symbol_Linked(const std::string & in_name, std::string & in_var, ARGS &&... args)
      : Symbol(in_name, std::forward<ARGS>(args)...), var(in_var) { ; }
    Symbol_Linked(const this_t &) = default;

    std::string GetTypename() const override { return "[LinkedString]"; }

    emp::Ptr<Symbol> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return emp::from_string<double>(var); }
    std::string AsString() const override { return var; }
    Symbol & SetValue(double in) override { var = emp::to_string(in); return *this; }
    Symbol & SetString(const std::string & in) override { var = in; return *this; }

    bool IsString() const override { return true; }

    bool CopyValue(const Symbol & in) override { var = in.AsString(); return true; }
  };

  /// Symbol can be linked to a pair of (Get and Set) functions
  /// rather than as direct variable.
  template <typename T>
  class Symbol_LinkedFunctions : public Symbol {
  private:
    std::function<T()> get_fun;
    std::function<void(const T &)> set_fun;
  public:
    using this_t = Symbol_LinkedFunctions<T>;

    template <typename... ARGS>
    Symbol_LinkedFunctions(const std::string & in_name,
                       std::function<T()> in_get,
                       std::function<void(const T &)> in_set,
                       ARGS &&... args)
      : Symbol(in_name, std::forward<ARGS>(args)...)
      , get_fun(in_get)
      , set_fun(in_set)
    { ; }
    Symbol_LinkedFunctions(const this_t &) = default;

    std::string GetTypename() const override { return "[Symbol_LinkedFunctions]"; }

    emp::Ptr<Symbol> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return emp::ToDouble( get_fun() ); }
    std::string AsString() const override { return emp::to_string( get_fun() ); }
    Symbol & SetValue(double in) override { set_fun(emp::FromDouble<T>(in)); return *this; }
    Symbol & SetString(const std::string & in) override {
      set_fun( emp::from_string<T>(in) );
      return *this;
    }

    bool IsNumeric() const override { return std::is_scalar_v<T>; }
    bool IsString() const override { return std::is_same<std::string, T>(); }

    bool CopyValue(const Symbol & in) override { SetString( in.AsString() ); return true; }
  };

}

#endif
