/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  EmplodeTypeBase.hpp
 *  @brief Base class for setting up custom types for use in scripting; usable throughout.
 *  @note Status: ALPHA
 */

#ifndef EMPLODE_TYPE_BASE_HPP
#define EMPLODE_TYPE_BASE_HPP

#include "emp/base/assert.hpp"

namespace emplode {

  class Symbol_Scope;
  class TypeInfo;

  enum class BaseType {
    INVALID = 0, 
    VOID,
    VALUE,
    STRING,
    STRUCT
  };

  class EmplodeTypeBase {
  protected:
    emp::Ptr<Symbol_Scope> cur_scope;
    emp::Ptr<TypeInfo> type_info_ptr;

    // Some special, internal variables associated with each object.
    bool _active=true;       ///< Should this object be used in the current run?
    std::string _desc="";    ///< Special description for this object.

  public:
    virtual ~EmplodeTypeBase() { }

    // Optional function to override to add configuration options associated with an object.
    virtual void SetupConfig() { };

    Symbol_Scope & GetScope() { emp_assert(!cur_scope.IsNull()); return *cur_scope; }
    const Symbol_Scope & GetScope() const { emp_assert(!cur_scope.IsNull()); return *cur_scope; }

    const TypeInfo & GetTypeInfo() const { return *type_info_ptr; }
  };

}

#endif
