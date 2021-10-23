/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigEntry_Function.hpp
 *  @brief Manages individual functions for config.
 *  @note Status: BETA
 */

#ifndef MABE_CONFIG_FUNCTION_H
#define MABE_CONFIG_FUNCTION_H

#include <string>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/tuple_utils.hpp"
#include "emp/meta/ValPack.hpp"

#include "ConfigEntry.hpp"
#include "ConfigTools.hpp"

namespace mabe {

  class ConfigEntry_Function : public ConfigEntry {
  private:
    using this_t = ConfigEntry_Function;
    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    using fun_t = std::function< entry_ptr_t( const emp::vector<entry_ptr_t> & ) >;
    fun_t fun;
    bool numeric_return = false;
    bool string_return = false;
    // size_t arg_count;

  public:
    template <typename FUN_T>
    ConfigEntry_Function(const std::string & _name,
                         FUN_T _fun,
                         const std::string & _desc,
                         emp::Ptr<ConfigEntry_Scope> _scope)
      : ConfigEntry(_name, _desc, _scope), fun(ConfigTools::WrapFunction(_name, _fun))
    {
      using return_t = typename emp::FunInfo<FUN_T>::return_t;
      numeric_return = std::is_scalar_v<return_t>;
      string_return = std::is_same<std::string, return_t>();
    }

    ConfigEntry_Function(const ConfigEntry_Function &) = default;
    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    bool IsFunction() const override { return true; }
    bool HasNumericReturn() const override { return numeric_return; }
    bool HasStringReturn() const override { return string_return; }

    entry_ptr_t Call( const emp::vector<entry_ptr_t> & args ) override { return fun(args); }
  };

}

#endif
