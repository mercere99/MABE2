/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  ConfigEntry_Linked.hpp
 *  @brief Manages a configuration entry linked to another variable or functions.
 *  @note Status: BETA
 */

#ifndef MABE_CONFIG_ENTRY_LINKED_HPP
#define MABE_CONFIG_ENTRY_LINKED_HPP

#include <type_traits>

#include "ConfigEntry.hpp"

namespace mabe {

  /// ConfigEntry can be linked directly to a real variable.
  template <typename T>
  class ConfigEntry_Linked : public ConfigEntry {
  private:
    T & var;
  public:
    using this_t = ConfigEntry_Linked<T>;

    template <typename... ARGS>
    ConfigEntry_Linked(const std::string & in_name, T & in_var, ARGS &&... args)
      : ConfigEntry(in_name, std::forward<ARGS>(args)...), var(in_var) { ; }
    ConfigEntry_Linked(const this_t &) = default;

    std::string GetTypename() const override {
      if constexpr (std::is_scalar_v<T>) return "Value";
      else return "Unknown";
    }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return (double) var; }
    std::string AsString() const override { return emp::to_string(var); }
    ConfigEntry & SetValue(double in) override { var = (T) in; return *this; }
    ConfigEntry & SetString(const std::string & in) override {
      var = emp::from_string<T>(in);
      return *this;
    }

    bool IsNumeric() const override { return std::is_scalar_v<T>; }
    bool IsBool() const override { return std::is_same<bool, T>(); }
    bool IsInt() const override { return std::is_same<int, T>(); }
    bool IsDouble() const override { return std::is_same<double, T>(); }

    bool CopyValue(const ConfigEntry & in) override { var = in.AsDouble(); return true; }
  };

  /// Specialization for ConfigEntry linked to a string variable.
  template <>
  class ConfigEntry_Linked<std::string> : public ConfigEntry {
  private:
    std::string & var;
  public:
    using this_t = ConfigEntry_Linked<std::string>;

    template <typename... ARGS>
    ConfigEntry_Linked(const std::string & in_name, std::string & in_var, ARGS &&... args)
      : ConfigEntry(in_name, std::forward<ARGS>(args)...), var(in_var) { ; }
    ConfigEntry_Linked(const this_t &) = default;

    std::string GetTypename() const override { return "String"; }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return emp::from_string<double>(var); }
    std::string AsString() const override { return var; }
    ConfigEntry & SetValue(double in) override { var = emp::to_string(in); return *this; }
    ConfigEntry & SetString(const std::string & in) override { var = in; return *this; }

    bool IsString() const override { return true; }

    bool CopyValue(const ConfigEntry & in) override { var = in.AsString(); return true; }
  };

  /// ConfigEntry can be linked to a pair of (Get and Set) functions
  /// rather than as direct variable.
  template <typename T>
  class ConfigEntry_LinkedFunctions : public ConfigEntry {
  private:
    std::function<T()> get_fun;
    std::function<void(const T &)> set_fun;
  public:
    using this_t = ConfigEntry_LinkedFunctions<T>;

    template <typename... ARGS>
    ConfigEntry_LinkedFunctions(const std::string & in_name,
                       std::function<T()> in_get,
                       std::function<void(const T &)> in_set,
                       ARGS &&... args)
      : ConfigEntry(in_name, std::forward<ARGS>(args)...)
      , get_fun(in_get)
      , set_fun(in_set)
    { ; }
    ConfigEntry_LinkedFunctions(const this_t &) = default;

    std::string GetTypename() const override { return "[[Function]]"; }

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    double AsDouble() const override { return emp::ToDouble( get_fun() ); }
    std::string AsString() const override { return emp::to_string( get_fun() ); }
    ConfigEntry & SetValue(double in) override { set_fun(emp::FromDouble<T>(in)); return *this; }
    ConfigEntry & SetString(const std::string & in) override {
      set_fun( emp::from_string<T>(in) );
      return *this;
    }

    bool IsNumeric() const override { return std::is_scalar_v<T>; }
    bool IsBool() const override { return std::is_same<bool, T>(); }
    bool IsInt() const override { return std::is_same<int, T>(); }
    bool IsDouble() const override { return std::is_same<double, T>(); }
    bool IsString() const override { return std::is_same<std::string, T>(); }

    bool CopyValue(const ConfigEntry & in) override { SetString( in.AsString() ); return true; }
  };

}

#endif
