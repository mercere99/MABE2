/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2024.
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

    static_assert(!std::is_const<T>(), "Variables cannot be const.");
    
  public:
    using this_t = Symbol_Linked<T>;

    template <typename... ARGS>
    Symbol_Linked(const emp::String & in_name, T & in_var, ARGS &&... args)
      : Symbol(in_name, std::forward<ARGS>(args)...), var(in_var) { ; }
    Symbol_Linked(const this_t &) = default;

    emp::String GetTypename() const override {
      if constexpr (std::is_scalar_v<T>) return "[LinkedValue]";
      else return "[Error:InvalidLinkedType]";
    }

    emp::Ptr<Symbol> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return (double) var; }
    emp::String AsString() const override { return emp::MakeString(var); }
    Symbol & SetValue(double in) override { var = (T) in; return *this; }
    Symbol & SetString(const emp::String & in) override {
      var = emp::from_string<T>(in);
      return *this;
    }

    bool HasValue() const override { return true; }

    bool IsNumeric() const override { return std::is_scalar_v<T>; }

    bool CopyValue(const Symbol & in) override { var = in.AsDouble(); return true; }
  };

  /// Specialization for Symbol linked to an emp::String variable.
  template <>
  class Symbol_Linked<emp::String> : public Symbol {
  private:
    emp::String & var;
  public:
    using this_t = Symbol_Linked<emp::String>;

    template <typename... ARGS>
    Symbol_Linked(const emp::String & in_name, emp::String & in_var, ARGS &&... args)
      : Symbol(in_name, std::forward<ARGS>(args)...), var(in_var) { ; }
    Symbol_Linked(const this_t &) = default;

    emp::String GetTypename() const override { return "[LinkedString]"; }

    emp::Ptr<Symbol> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return var.AsDouble(); }
    emp::String AsString() const override { return var; }
    Symbol & SetValue(double in) override { var.Set(in); return *this; }
    Symbol & SetString(const emp::String & in) override { var = in; return *this; }

    bool HasValue() const override { return true; }

    bool IsString() const override { return true; }

    bool CopyValue(const Symbol & in) override { var = in.AsString(); return true; }
  };

  /// Specialization for Symbol linked to an std::string variable.
  template <>
  class Symbol_Linked<std::string> : public Symbol {
  private:
    std::string & var;
  public:
    using this_t = Symbol_Linked<std::string>;

    template <typename... ARGS>
    Symbol_Linked(const emp::String & in_name, std::string & in_var, ARGS &&... args)
      : Symbol(in_name, std::forward<ARGS>(args)...), var(in_var) { ; }
    Symbol_Linked(const this_t &) = default;

    emp::String GetTypename() const override { return "[LinkedString]"; }

    emp::Ptr<Symbol> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return std::stod(var); }
    emp::String AsString() const override { return var; }
    Symbol & SetValue(double in) override { var = emp::MakeString(in); return *this; }
    Symbol & SetString(const emp::String & in) override { var = in; return *this; }

    bool HasValue() const override { return true; }

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
    Symbol_LinkedFunctions(const emp::String & in_name,
                       std::function<T()> in_get,
                       std::function<void(const T &)> in_set,
                       ARGS &&... args)
      : Symbol(in_name, std::forward<ARGS>(args)...)
      , get_fun(in_get)
      , set_fun(in_set)
    { ; }
    Symbol_LinkedFunctions(const this_t &) = default;

    emp::String GetTypename() const override { return "[Symbol_LinkedFunctions]"; }

    emp::Ptr<Symbol> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return emp::ToDouble( get_fun() ); }
    emp::String AsString() const override { return emp::MakeString( get_fun() ); }
    Symbol & SetValue(double in) override { set_fun(emp::FromDouble<T>(in)); return *this; }
    Symbol & SetString(const emp::String & in) override {
      set_fun( emp::from_string<T>(in) );
      return *this;
    }

    bool HasValue() const override { return true; }

    bool IsNumeric() const override { return std::is_scalar_v<T>; }
    bool IsString() const override { return std::is_same<std::string, T>() || std::is_same<emp::String, T>(); }

    bool CopyValue(const Symbol & in) override {
      if (in.IsNumeric()) SetValue(in.AsDouble());
      else SetString(in.AsString());
      return true;
    }
  };

}

#endif
