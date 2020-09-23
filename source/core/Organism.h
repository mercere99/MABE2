/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  Organism.h
 *  @brief A base class for all organisms in MABE.
 *  @note Status: ALPHA
 *
 *  All organism types in MABE must have mabe::Organism as it ultimate base class.  A helper
 *  template mabe::OrganismTeplate<ORG_T> is derived from mabe::Organism and should be used as
 *  the more immeidate base class for any user-defined organism types.  Providing this template
 *  with your new organism type as ORG_T will setup type-specific return values for ease of use.
 *
 *  All interactions between an organism and its environment are mediated through the Organism's
 *  DataMap.  The configuration files need to be setup to ensure that environments and organisms
 *  agree on the input values, the output values, and use of any type adaptors.
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

    /// The class below is a placeholder for storing any manager-specific data that the organims
    /// should have access to.  A derived organism class merely needs to shadow this one in order
    /// to include specialized data.
    struct ManagerData {
    };

    bool HasVar(const std::string & name) const { return data_map.HasName(name); }
    template <typename T> T & GetVar(const std::string & name) { return data_map.Get<T>(name); }
    template <typename T> const T & GetVar(const std::string & name) const {
      return data_map.Get<T>(name);
    }
    template <typename T> T & GetVar(size_t id) { return data_map.Get<T>(id); }
    template <typename T> const T & GetVar(size_t id) const { return data_map.Get<T>(id); }

    template <typename T>
    void SetVar(const std::string & name, const T & value) {
      if (data_map.HasName(name) == false) data_map.AddVar<T>(name, value);
      else data_map.Set<T>(name, value);
    }

    template <typename T>
    void SetVar(size_t id, const T & value) {
      emp_assert(data_map.HasID(id), id);
      data_map.Set<T>(id, value);
    }

    emp::DataMap & GetDataMap() { return data_map; }
    const emp::DataMap & GetDataMap() const { return data_map; }

    void SetDataMap(emp::DataMap & in_dm) { data_map = in_dm; }

    bool HasTraitID(size_t id) const { return data_map.HasID(id); }
    bool HasTrait(const std::string & name) const { return data_map.HasName(name); }
    template <typename T>
    bool TestTraitType(size_t id) const { return data_map.IsType<T>(id); }
    template <typename T>
    bool TestTraitType(const std::string & name) const { return data_map.IsType<T>(name); }

    size_t GetTraitID(const std::string & name) const { return data_map.GetID(name); }

    template <typename T>
    T & GetTrait(size_t id) { return data_map.Get<T>(id); }

    template <typename T>
    const T & GetTrait(size_t id) const { return data_map.Get<T>(id); }

    template <typename T>
    T & GetTrait(const std::string & name) { return data_map.Get<T>(name); }

    template <typename T>
    const T & GetTrait(const std::string & name) const { return data_map.Get<T>(name); }

    template <typename T>
    T & SetTrait(size_t id, const T & val) { return data_map.Set<T>(id, val); }

    template <typename T>
    T & SetTrait(const std::string & name, const T & val) { return data_map.Set<T>(name, val); }

    emp::TypeID GetTraitType(size_t id) const { return data_map.GetType(id); }
    emp::TypeID GetTraitType(const std::string & name) const { return data_map.GetType(name); }

    double GetTraitAsDouble(size_t id) const { return data_map.GetAsDouble(id); }

    double GetTraitAsDouble(size_t trait_id, emp::TypeID type_id) const {
      return data_map.GetAsDouble(trait_id, type_id);
    }

    std::string GetTraitAsString(size_t id) const { return data_map.GetAsString(id); }

    std::string GetTraitAsString(size_t trait_id, emp::TypeID type_id) const {
      return data_map.GetAsString(trait_id, type_id);
    }


    /// Test if this organism represents an empy cell.
    virtual bool IsEmpty() const noexcept { return false; }


    // ------------------------------------------
    // ------   Functions for overriding   ------
    // ------------------------------------------


    /// Create an exact duplicate of this organism.
    /// @note We MUST be able to make a copy of organisms for MABE to function.  If this function
    /// is not overridden, try to the equivilent function in the organism manager.
    [[nodiscard]] virtual emp::Ptr<Organism> Clone() const { return manager.CloneOrganism(*this); }

    /// Modify this organism based on configured mutation parameters.
    /// @note For evolution to function, we need to be able to mutate offspring.
    virtual size_t Mutate(emp::Random & random) { return manager.Mutate(*this, random); }

    /// Merge this organism's genome with that of another organism to produce an offspring.
    /// @note Required for basic sexual recombination to work.
    [[nodiscard]] virtual emp::Ptr<Organism>
    Recombine(emp::Ptr<Organism> parent2, emp::Random & random) const {
      return manager.Recombine(*this, parent2, random);
    }

    /// Merge this organism's genome with that of a variable number of other organisms to produce
    /// a variable number of offspring.
    /// @note More flexible version of recombine (allowing many parents and/or many offspring),
    /// but also slower.
    [[nodiscard]] virtual emp::vector<emp::Ptr<Organism>>
    Recombine(emp::vector<emp::Ptr<Organism>> other_parents, emp::Random & random) const {
      return manager.Recombine(*this, other_parents, random);
    }

    /// Produce an asexual offspring WITH MUTATIONS.  By default, use Clone() and then Mutate().
    [[nodiscard]] virtual emp::Ptr<Organism> MakeOffspring(emp::Random & random) const {
      emp::Ptr<Organism> offspring = Clone();
      offspring->Mutate(random);
      return offspring;
    }

    /// Produce an sexual (two parent) offspring WITH MUTATIONS.  By default, use Recombine() and
    /// then Mutate().
    [[nodiscard]] virtual emp::Ptr<Organism>
    MakeOffspring(emp::Ptr<Organism> parent2, emp::Random & random) const {
      emp::Ptr<Organism> offspring = Recombine(parent2, random);
      offspring->Mutate(random);
      return offspring;
    }

    /// Produce one or more offspring from multiple parents WITH MUTATIONS.  By default, use
    /// Recombine() and then Mutate().
    [[nodiscard]] virtual emp::vector<emp::Ptr<Organism>> 
    MakeOffspring(emp::vector<emp::Ptr<Organism>> other_parents, emp::Random & random) const {
      emp::vector<emp::Ptr<Organism>> all_offspring = Recombine(other_parents, random);
      for (auto offspring : all_offspring) offspring->Mutate(random);
      return all_offspring;
    }

    /// Convert this organism into a string of characters.
    /// @note Required if we are going to print organisms to screen or to file).  If this function
    /// is not overridden, try to the equivilent function in the organism manager.
    virtual std::string ToString() const { return manager.OrgToString(*this); }

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


  // Pre-declare OrganismManager to allow for conversions.
  template <typename ORG_T> class OrganismManager;


  /// Below is a specialty Organism type that uses "curiously recursive templates" to fill out
  /// more default functionality for when you know the derived organism type.  Specifically,
  /// it should be used as the base class for any derived organism types.
  template <typename ORG_T>
  class OrganismTemplate : public Organism {
  public:
    OrganismTemplate(ModuleBase & _man) : Organism(_man) { ; }

    using org_t = ORG_T;
    using manager_t = OrganismManager<ORG_T>;

    /// Get the manager for this type of organism.
    manager_t & GetManager() {
      return (manager_t &) Organism::GetManager();
    }
    const manager_t & GetManager() const {
      return (const manager_t &) Organism::GetManager();
    }

    auto & SharedData() { return GetManager().data; }
    const auto & SharedData() const { return GetManager().data; }
  };

}
#endif
