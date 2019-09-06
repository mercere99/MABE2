/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  SymbolTable.h
 *  @brief Manages all types and symbols in MABE runs.
 *  @note Status: ALPHA
 **/

#ifndef MABE_SYMBOL_TABLE_H
#define MABE_SYMBOL_TABLE_H

#include "base/assert.h"
#include "base/map.h"
#include "meta/TypeID.h"
#include "tools/map_utils.h"
#include "tools/string_utils.h"

namespace mabe {

  /// General type information for variables in a scripting language.
  class TypeInfo {
  private:
    std::string name;

  public:
    TypeInfo(const std::string & in_name) : name(in_name) { ; }
  };

  /// Specialty type information for when we are working with strings.
  class TypeInfo_String : public TypeInfo {
  private:
    std::string name;

  public:
    TypeInfo_String(const std::string & in_name) : TypeInfo(in_name) { ; }
  };

  /// Specialty type information for when we are working with numeric values.
  template <typename T>
  class TypeInfo_Numeric : public TypeInfo {
  private:
    std::string name;

  public:
    TypeInfo_Numeric(const std::string & in_name) : TypeInfo(in_name) { ; }
  };

  /// Specialty type information for when we are working with a more complex structure.
  template <typename T>
  class TypeInfo_Struct : public TypeInfo {
  private:
    std::string name;

  public:
    TypeInfo_Struct(const std::string & in_name) : TypeInfo(in_name) { ; }
  };

  class SymbolInfo {
  private:
    std::string name;
    const TypeInfo & type;

  public:
    SymbolInfo(const std::string & in_name,
               const TypeInfo & in_type)
    : name(in_name), type(in_type) { ; }
    ~SymbolInfo() { ; }
  };

  class SymbolTable {
  private:
    emp::map<std::string, emp::Ptr<TypeInfo>> type_map;
  public:
    SymbolTable() { ; }
    ~SymbolTable() { ; }

    // --- Simple type handling ---

    TypeInfo & GetType(const std::string & name) {
      emp_assert(HasType(name), "Trying to get type that does not exist.", name);
      return *type_map[name];
    }

    bool HasType(const std::string & name) { return emp::Has(type_map, name); }

    template <typename T>
    TypeInfo & AddType(const std::string & name) {
      emp_assert(!HasType(name), "Trying to add type that already exists.", name);
      type_map[name] = new TypeInfo_Struct<T>(name);
      return *type_map[name];
    }

    // --- Simple variable handling ---

    void AddSymbol() { ; }
  };

}

#endif