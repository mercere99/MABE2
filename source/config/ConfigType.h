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
    VOID,
    VALUE,
    STRING,
    STRUCT
  };

  class ConfigType {
  private:
    BaseType base_type; ///< What is the base type?
    size_t array_depth; ///< How nested is the array?

  public:
    ConfigType(BaseType in_base, size_t in_adepth=0)
      : base_type(in_base), array_depth(in_adepth) { ; }
    ConfigType(ConfigType &) = default;

    ConfigType & operator=(ConfigType &) = default;

    bool operator==(const ConfigType & in) {
      return base_type == in.base_type && array_depth == in.array_depth;
    }
    bool operator!=(const ConfigType & in) {
      return base_type != in.base_type || array_depth != in.array_depth;
    }

    bool IsBaseType() const noexcept { return array_depth == 0; }
    bool IsArray() const noexcept { return array_depth > 0; }

    bool IsVoid() const noexcept { return IsBaseType() && base_type == BaseType::VOID; };
    bool IsValue() const noexcept { return IsBaseType() && base_type == BaseType::VALUE; };
    bool IsString() const noexcept { return IsBaseType() && base_type == BaseType::STRING; };
    bool IsStruct() const noexcept { return IsBaseType() && base_type == BaseType::STRUCT; };

    BaseType GetBaseType() const noexcept { return base_type; }
    size_t GetArrayDepth() const noexcept { return array_depth; }

    /// Get the type of each element if this is an array.  Return void otherwise.
    ConfigType GetElementType() const noexcept {
      if (array_depth == 0) return BaseType::VOID;
      return ConfigType(base_type, array_depth-1);
    }

    /// Get the type of an array of this type.
    ConfigType GetArrayType() const noexcept { return ConfigType(base_type, array_depth+1); }
  };

}

#endif