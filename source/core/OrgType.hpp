/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  OrgType.hpp
 *  @brief A base class for all organism components, with facilities for replication.
 *  @note Status: ALPHA
 */

#ifndef MABE_ORG_TYPE_HPP
#define MABE_ORG_TYPE_HPP

#include "ModuleBase.hpp"

namespace mabe {

  class Module;

  // A class type managed by a ManagerModule.
  class OrgType {
  protected:
    ModuleBase & manager;    ///< Manager for the specific organism type

  public:
    OrgType(ModuleBase & _man) : manager(_man) { ; }
    virtual ~OrgType() { ; }

    /// Get the manager for this type of organism.
    Module & GetManager() { return (Module&) manager; }
    const Module & GetManager() const { return (Module&) manager; }

    /// The class below is a placeholder for storing any manager-specific data that the organisms
    /// should have access to.  A derived organism class merely needs to shadow this one in order
    /// to include specialized data.
    struct ManagerData {
    };


    // ------------------------------------------
    // ------   Functions for overriding   ------
    // ------------------------------------------


    /// Create an exact duplicate of this organism.
    /// @note We MUST be able to make a copy of organisms for MABE to function.  If this function
    /// is not overridden, the organism manager (which knows the derived type) will try to make a
    /// clone using the copy constructor.
    [[nodiscard]] virtual emp::Ptr<OrgType> Clone() const { return manager.CloneObject(*this); }

    /// Modify this organism based on configured mutation parameters.
    /// @note For evolution to function, we need to be able to mutate offspring.
    virtual size_t Mutate(emp::Random & random) = 0;

    /// Merge this organism's genome with that of another organism to produce an offspring.
    /// @note Required for basic sexual recombination to work.
    [[nodiscard]] virtual emp::Ptr<OrgType>
    Recombine(emp::Ptr<OrgType> /* parent2 */, emp::Random & /* random */) const {
      emp_assert(false, "Recombine() must be overridden for it to work.");
      return nullptr;
    }

    /// Merge this organism's genome with that of a variable number of other organisms to produce
    /// a variable number of offspring.
    /// @note More flexible version of recombine (allowing many parents and/or many offspring),
    /// but also slower.
    [[nodiscard]] virtual emp::vector<emp::Ptr<OrgType>>
    Recombine(emp::vector<emp::Ptr<OrgType>> /*other_parents*/, emp::Random & /*random*/) const {
      emp_assert(false, "Recombine() must be overridden for it to work.");
      return emp::vector<emp::Ptr<OrgType>>();
    }

    /// Produce an asexual offspring WITH MUTATIONS.  By default, use Clone() and then Mutate().
    [[nodiscard]] virtual emp::Ptr<OrgType> MakeOffspring(emp::Random & random) const {
      emp::Ptr<OrgType> offspring = Clone();
      offspring->Mutate(random);
      return offspring;
    }

    /// Produce an sexual (two parent) offspring WITH MUTATIONS.  By default, use Recombine() and
    /// then Mutate().
    [[nodiscard]] virtual emp::Ptr<OrgType>
    MakeOffspring(emp::Ptr<OrgType> parent2, emp::Random & random) const {
      emp::Ptr<OrgType> offspring = Recombine(parent2, random);
      offspring->Mutate(random);
      return offspring;
    }

    /// Produce one or more offspring from multiple parents WITH MUTATIONS.  By default, use
    /// Recombine() and then Mutate().
    [[nodiscard]] virtual emp::vector<emp::Ptr<OrgType>> 
    MakeOffspring(emp::vector<emp::Ptr<OrgType>> other_parents, emp::Random & random) const {
      emp::vector<emp::Ptr<OrgType>> all_offspring = Recombine(other_parents, random);
      for (auto offspring : all_offspring) offspring->Mutate(random);
      return all_offspring;
    }

    /// Convert this organism into a string of characters.
    /// @note Required if we are going to print organisms to screen or to file).  If this function
    /// is not overridden, try to the equivalent function in the organism manager.
    virtual std::string ToString() const { return "__unknown__"; }

    /// By default print an organism by triggering it's ToString() function.
    std::ostream & Print(std::ostream & os) const {
      os << ToString();
      return os;
    }

    /// Completely randomize a new organism (typically for initialization)
    virtual void Randomize(emp::Random & /*random*/) {
      emp_assert(false, "Randomize() must be overridden before it can be called.");
    }

    /// Setup a new organism from scratch; by default just randomize.
    virtual void Initialize(emp::Random & random) { Randomize(random); }

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
