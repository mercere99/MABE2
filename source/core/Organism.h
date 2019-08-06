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

#include "OrganismManager.h"

namespace mabe {

  class Organism {
  protected:
    emp::VarMap var_map;                    ///< Map of all dynamic variables assigned to organism
    emp::Ptr<const OrganismManager> manager_ptr;  ///< Pointer the the specific organism type

  public:
    Organism(emp::Ptr<const OrganismManager> _ptr) : manager_ptr(_ptr) { ; }
    virtual ~Organism() { ; }

    const OrganismManager & GetManager() { emp_assert(manager_ptr); return *manager_ptr; }
    const OrganismManager & GetManager() const { emp_assert(manager_ptr); return *manager_ptr; }

    bool HasVar(const std::string & name) const { return var_map.Has(name); }
    template <typename T> T & GetVar(const std::string & name) { return var_map.Get<T>(name); }
    template <typename T> const T & GetVar(const std::string & name) const { return var_map.Get<T>(name); }

    template <typename T>
    void SetVar(const std::string & name, const T & value) {
      var_map.Set(name, value);
    }

    /// Test if this organism represents an empy cell.
    virtual bool IsEmpty() const noexcept { return false; }

    // --- Functions for overriding ---

    /// We MUST be able to make a copy of organisms for MABE to function.  If this function
    /// is not overridden, try to the equivilent function in the organism manager.
    virtual emp::Ptr<Organism> Clone() const { return manager_ptr->CloneOrganism(*this); }

    /// If we are going to print organisms (to screen or file) we need to be able to convert
    /// them to strings.  If this function is not overridden, try to the equivilent function
    /// in the organism manager.
    virtual std::string ToString() { return manager_ptr->ToString(*this); }

    /// For evolution to function, we need to be able to mutate offspring.
    virtual size_t Mutate(emp::Random & random) { return manager_ptr->Mutate(*this, random); }

    /// Completely randomize a new organism (typically for initialization)
    virtual void Randomize(emp::Random & random) { manager_ptr->Randomize(*this, random); }

    /// Generate an output and place it in the VarMap under the provided name (default = "result").
    /// Arguments are the output name int he VarMap and the output ID.
    virtual void GenerateOutput(const std::string & ="result", size_t=0) { ; }

    /// Request output type (multiple types are possible); default to unknown.
    /// Argument is the output ID.
    virtual emp::TypeID GetOutputType(size_t=0) { return emp::TypeID(); }


    /// --- Extra functions for when this is used a a prototype organism ---
    
    /// Setup organism-specific configuration options.
    virtual void SetupConfig(ConfigScope & config_scope) { ; }

  };

}
#endif
