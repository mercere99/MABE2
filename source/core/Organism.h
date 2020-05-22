/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  Organism.h
 *  @brief A base class for all organisms in MABE.
 *  @note Status: ALPHA
 *
 *  All organism types in MABE must override the mabe::Organism class and provide a valid
 *  OrganismManager (see OrganismManager.h for more information).
 *
 *  All interactions between an organism and its environment should be mediated through the
 *  organism's DataMap.  Environments should setup INPUT VALUES in the DataMap before 
 *  the population is executed.  During (or at the end of) execution, organisms should
 *  write out any OUTPUT_VALUES to the data map.  The configuration files should be used
 *  to ensure that the setting names align correctly, and type adaptors can be added in
 *  to similarly ensure that types correctly match up.
 * 
 *  If an environment wants to allow ACTIONS to occur during execution, it can provide
 *  callback functions to the organisms in the appropriate OrganismManager DataMap.  If
 *  the environment wants to indicate EVENTS that occur during an organism's lifetime,
 *  it can find the appropriate function to call in the manager's DataMap.
 * 
 */

#ifndef MABE_ORGANISM_H
#define MABE_ORGANISM_H

#include "base/assert.h"
#include "base/vector.h"
#include "data/DataMap.h"
#include "meta/TypeID.h"
#include "tools/BitVector.h"
#include "tools/string_utils.h"

#include "ModuleBase.h"

namespace mabe {

  class Module;

  class Organism {
  private:
    emp::DataMap data_map;   ///< Dynamic variables assigned to organism
    ModuleBase & manager;    ///< Manager for the specific organism type

  public:
    Organism(ModuleBase & _man) : manager(_man) { ; }
    virtual ~Organism() { ; }

    /// Get the manager for this type of organism.
    Module & GetManager() { return (Module&) manager; }
    const Module & GetManager() const { return (Module&) manager; }

    bool HasVar(const std::string & name) const { return data_map.HasName(name); }
    template <typename T> T & GetVar(const std::string & name) { return data_map.Get<T>(name); }
    template <typename T> const T & GetVar(const std::string & name) const {
      return data_map.Get<T>(name);
    }

    template <typename T>
    void SetVar(const std::string & name, const T & value) {
      if (data_map.HasName(name) == false) data_map.AddVar<T>(name, value);
      else data_map.Set<T>(name, value);
    }

    emp::DataMap & GetDataMap() { return data_map; }
    const emp::DataMap & GetDataMap() const { return data_map; }

    void SetDataMap(emp::DataMap & in_dm) { data_map = in_dm; }

    /// Test if this organism represents an empy cell.
    virtual bool IsEmpty() const noexcept { return false; }

    // --- Functions for overriding ---

    /// Create an exact duplicate of this organism.
    /// @note We MUST be able to make a copy of organisms for MABE to function.  If this function
    /// is not overridden, try to the equivilent function in the organism manager.
    virtual emp::Ptr<Organism> Clone() const { return manager.CloneOrganism(*this); }

    /// Merge this organism's genome with that of another organism to produce an offspring.
    /// @note Required for basic sexual recombination to work.
    virtual emp::Ptr<Organism> Recombine(emp::Ptr<Organism> parent2) const {
      // @CAO: Implement this
      return nullptr;
    }

    /// Merge this organism's genome with that of a variable number of other organisms to produce
    /// a variable number of offspring.
    /// @note More flexible version of recombine (allowing many parents or many offspring), but
    /// also slower.
    virtual emp::vector<emp::Ptr<Organism>> Recombine(emp::vector<emp::Ptr<Organism>> other_parents) const {
      // @CAO: Implement this
      return emp::vector< emp::Ptr<Organism> >();
    }

    /// Convert this organism into a string of characters.
    /// @note Required if we are going to print organisms to screen or to file).  If this function
    /// is not overridden, try to the equivilent function in the organism manager.
    virtual std::string ToString() { return manager.OrgToString(*this); }

    /// Modify this organism based on configured mutation parameters.
    /// @note For evolution to function, we need to be able to mutate offspring.
    virtual size_t Mutate(emp::Random & random) { return manager.Mutate(*this, random); }

    /// Completely randomize a new organism (typically for initialization)
    virtual void Randomize(emp::Random & random) { manager.Randomize(*this, random); }

    /// Run the organism to generate an output in the pre-configured data_map entries.
    virtual void GenerateOutput() { ; }

    /// Run the organisms a single time step; only implemented for continuous execution organisms.
    virtual bool ProcessStep() { return false; }
 
    // virtual bool AddEvent(const std::string & event_name, int event_id) { return false; }
    // virtual void TriggerEvent(int) { ; }


    ///
    /// --- Extra functions for when this is used as a PROTOTYPE ORGANISM only! ---
    ///

    /// Setup organism-specific configuration options.
    virtual void SetupConfig() { ; }

    /// Setup organism-specific traits.
    virtual void SetupModule() { ; }

  };

}
#endif
