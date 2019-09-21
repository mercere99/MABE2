/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigType.h
 *  @brief Setup types for use in scripting.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_TYPE_H
#define MABE_CONFIG_TYPE_H

#include "base/assert.h"

namespace mabe {

  enum class BaseType {
    INVALID = 0, 
    VOID,
    VALUE,
    STRING,
    STRUCT
  };

  class ConfigScope;

  // Abstract base class for types that we want to be used for scripting..
  struct ConfigType {
    virtual void SetupConfig(ConfigScope & scope) = 0;
    virtual ~ConfigType() = 0;
  };
}

#endif