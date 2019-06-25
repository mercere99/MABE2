/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Organism.h
 *  @brief A base class for all organisms in MABE.
 *  @note Status: ALPHA
 */

#ifndef MABE_ORGANISM_H
#define MABE_ORGANISM_H

#include "base/assert.h"
#include "data/VarMap.h"
#include "meta/TypeID.h"
#include "tools/string_utils.h"

class OrganismType;

namespace mabe {

  class Organism {
  protected:
    emp::VarMap var_map;             ///< Map of all dynamic variables assigned to an organism.
    emp::Ptr<OrganismType> type_ptr;  ///< Pointer the the specific organism type.

  public:
    virtual ~Organism() { ; }

    OrganismType & GetType() { return *type_ptr; }
    const OrganismType & GetType() const { return *type_ptr; }

    bool HasVar(const std::string & name) const { return var_map.Has(name); }
    template <typename T> T & GetVar(const std::string & name) { return var_map.Get<T>(name); }
    template <typename T> const T & GetVar(const std::string & name) const { return var_map.Get<T>(name); }

    template <typename T>
    void SetVar(const std::string & name, const T & value) {
      var_map.Set(name, value);
    }

    // --- Functions for overriding ---

    /// We MUST be able to make a copy of organisms for MABE to function.
    virtual emp::Ptr<Organism> Clone() = 0;  

    /// If we are going to print organisms (to screen or file) we need to be able to convert
    /// them to strings.
    virtual std::string ToString() { return "__unknown__"; }

    /// For evolution to function, we need to be able to mutate offspring.
    virtual int Mutate(emp::Random &) { emp_assert(false, "No default Mutate() available."); return -1; }

    /// Completely randomize a new organism (typically for initialization)
    virtual int Randomize(emp::Random &) { emp_assert(false, "No default Randomize() available."); return -1; }

    /// Generate an output and place it in the VarMap under the provided name (default = "result").
    /// Arguments are the output name int he VarMap and the output ID.
    virtual void GenerateOutput(const std::string & ="result", size_t=0) { }

    /// Request output type (multiple types are possible); default to unknown.
    /// Argument is the output ID.
    virtual emp::TypeID GetOutputType(size_t=0) { return emp::TypeID(); }
  };

}
#endif
