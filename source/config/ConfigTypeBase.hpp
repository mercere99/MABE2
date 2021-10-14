/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  ConfigTypeBase.hpp
 *  @brief Base class for setting up custom types for use in scripting; usable throughout.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_TYPE_BASE_HPP
#define MABE_CONFIG_TYPE_BASE_HPP

#include "emp/base/assert.hpp"

namespace mabe {

  class ConfigEntry_Scope;
  class ConfigTypeInfo;

  enum class BaseType {
    INVALID = 0, 
    VOID,
    VALUE,
    STRING,
    STRUCT
  };

  class ConfigTypeBase {
  protected:
    emp::Ptr<ConfigEntry_Scope> cur_scope;
    emp::Ptr<ConfigTypeInfo> type_info_ptr;

    // Some special, internal variables associated with each object.
    bool _active=true;       ///< Should this object be used in the current run?
    std::string _desc="";    ///< Special description for this object.

  public:
    virtual ~ConfigTypeBase() { }

    // Optional function to override to add configuration options associated with an object.
    virtual void SetupConfig() { };

    ConfigEntry_Scope & GetScope() { emp_assert(!cur_scope.IsNull()); return *cur_scope; }
    const ConfigEntry_Scope & GetScope() const { emp_assert(!cur_scope.IsNull()); return *cur_scope; }

    const ConfigTypeInfo & GetTypeInfo() const { return *type_info_ptr; }
  };

}

#endif
