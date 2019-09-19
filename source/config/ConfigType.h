/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigType.h
 *  @brief Maintains information about a single type.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_TYPE_H
#define MABE_CONFIG_TYPE_H

#include "base/assert.h"
#include "base/vector.h"

namespace mabe {

  enum class BaseType {
    INVALID = 0, 
    VOID,
    VALUE,
    STRING,
    STRUCT
  };

}

#endif