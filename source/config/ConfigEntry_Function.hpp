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
    using entry_vector_t = emp::vector<entry_ptr_t>;
    using fun_t = std::function< entry_ptr_t( const entry_vector_t & ) >;
    fun_t fun;
    bool numeric_return = false;
    bool string_return = false;
    // size_t arg_count;

  public:
    template <typename RETURN_T, typename... ARGS>
    ConfigEntry_Function(const std::string & _name,
                   std::function<RETURN_T(ARGS...)> _fun,
                   const std::string & _desc,
                   emp::Ptr<ConfigEntry_Scope> _scope)
      : ConfigEntry(_name, _desc, _scope) { SetFunction(_fun); }

    ConfigEntry_Function(const ConfigEntry_Function &) = default;

    emp::Ptr<ConfigEntry> Clone() const override { return emp::NewPtr<this_t>(*this); }

    bool IsFunction() const override { return true; }
    bool HasNumericReturn() const override { return numeric_return; }
    bool HasStringReturn() const override { return string_return; }

    /// Setup a function that takes AT LEAST ONE argument.
    template <typename FUN>
    void SetFunction( FUN in_fun ) {
      fun = ConfigTools::WrapFunction(name, in_fun);
    }

    entry_ptr_t Call( const emp::vector<entry_ptr_t> & args ) override { return fun(args); }

  };

}

#endif
