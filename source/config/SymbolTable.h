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

  class TypeInfo {
  };

  class SymbolInfo {
  private:
    const TypeInfo & type;

  protected:
    SymbolInfo(const TypeInfo & in_type) : type(in_type) { ; }
    ~SymbolInfo() { ; }
  };

  class SymbolTable {
  private:
    emp::map<std::string, TypeInfo> type_map;
  public:
    SymbolTable() { ; }
    ~SymbolTable() { ; }

    // Simple type handling.
    TypeInfo & AddType(const std::string & name) { return type_map[name]; }
    TypeInfo & GetType(const std::string & name) { return type_map[name]; }
    bool HasType(const std::string & name) { return emp::Has(type_map, name); }

    void AddSymbol() { ; }
  };

}

#endif