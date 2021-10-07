/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  Organism.hpp
 *  @brief A base class for all organisms in MABE.
 *  @note Status: ALPHA
 *
 *  All organism types or organism component types (e.g., brains or genomes) that can be
 *  individually configured in MABE must have mabe::OrgType as its ultimate base class.  A helper
 *  template mabe::OrganismTemplate<ORG_T> is derived from mabe::OrgType and should be used as
 *  the more immeidate base class for any user-defined organism types.  Providing this template
 *  with your new organism type as ORG_T will setup type-specific return values for ease of use.
 *
 *  All interactions between an organism and its environment are mediated through the Organism's
 *  DataMap.  The configuration files need to be setup to ensure that environments and organisms
 *  agree on the input values, the output values, and use of any type adaptors.
 * 
 *  If an environment wants to allow ACTIONS to occur during execution, it can provide callback
 *  functions to the organisms in the appropriate OrganismManager DataMap.  If the environment
 *  wants to indicate EVENTS that occur during an organism's lifetime, it can find the appropriate
 *  function to call in the manager's DataMap.
 * 
 */

#ifndef MABE_ORGANISM_H
#define MABE_ORGANISM_H

#include "emp/base/assert.hpp"
#include "emp/base/vector.hpp"
#include "emp/data/AnnotatedType.hpp"
#include "emp/tools/string_utils.hpp"

#include "OrgType.hpp"

namespace mabe {

  class Organism : public OrgType, public emp::AnnotatedType {
  public:
    Organism(ModuleBase & _man) : OrgType(_man) { ; }
    virtual ~Organism() {}

    /// Test if this organism represents an empty cell.
    virtual bool IsEmpty() const noexcept { return false; }

    /// Specialty version of Clone to return an Organism type.
    [[nodiscard]] emp::Ptr<Organism> CloneOrganism() const {
      return OrgType::Clone().DynamicCast<Organism>();
    }

    [[nodiscard]] emp::Ptr<Organism>
    RecombineOrganisms(emp::Ptr<Organism> parent2, emp::Random & random) const {
      return OrgType::Recombine(parent2, random).DynamicCast<Organism>();
    }

    // @CAO: Need to clean this one up...
    [[nodiscard]] emp::vector<emp::Ptr<OrgType>>
    RecombineOrganisms(emp::vector<emp::Ptr<OrgType>> other_parents, emp::Random & random) const {
      return OrgType::Recombine(other_parents, random);
    }

    /// Produce an asexual offspring WITH MUTATIONS.  By default, use Clone() and then Mutate().
    [[nodiscard]] emp::Ptr<Organism> MakeOffspringOrganism(emp::Random & random) const {
      return OrgType::MakeOffspring(random).DynamicCast<Organism>();
    }

    /// Produce an sexual (two parent) offspring WITH MUTATIONS.  By default, use Recombine() and
    /// then Mutate().
    [[nodiscard]] emp::Ptr<Organism>
    MakeOffspringOrganism(emp::Ptr<Organism> parent2, emp::Random & random) const {
      return OrgType::MakeOffspring(parent2, random).DynamicCast<Organism>();
    }

    // @CAO: Need to clean this one up to use Organism...
    /// Produce one or more offspring from multiple parents WITH MUTATIONS.  By default, use
    /// Recombine() and then Mutate().
    [[nodiscard]] emp::vector<emp::Ptr<OrgType>> 
    MakeOffspringOrganisms(emp::vector<emp::Ptr<OrgType>> other_parents, emp::Random & random) const {
      return OrgType::MakeOffspring(other_parents, random);
    }




    // -- Also deal with some deprecated functionality... --

    [[deprecated("Use OrgType::HasTrait() instead of OrgType::HasVar()")]]
    bool HasVar(const std::string & name) const { return HasTrait(name); }
    template <typename T>
    [[deprecated("Use OrgType::GetTrait() instead of OrgType::GetVar()")]]
    T & GetVar(const std::string & name) { return GetTrait<T>(name); }
    template <typename T>
    [[deprecated("Use OrgType::GetTrait() instead of OrgType::GetVar()")]]
    const T & GetVar(const std::string & name) const { return GetTrait<T>(name); }
    template <typename T>
    [[deprecated("Use OrgType::GetTrait() instead of OrgType::GetVar()")]]
    T & GetVar(size_t id) { return GetTrait<T>(id); }
    template <typename T>
    [[deprecated("Use OrgType::GetTrait() instead of OrgType::GetVar()")]]
    const T & GetVar(size_t id) const { return GetTrait<T>(id); }

    template <typename T>
    [[deprecated("Use OrgType::SetTrait() instead of OrgType::SetVar()")]]
    void SetVar(const std::string & name, const T & value) { SetTrait(name, value); }

    template <typename T>
    [[deprecated("Use OrgType::SetTrait() instead of OrgType::SetVar()")]]
    void SetVar(size_t id, const T & value) { SetTrait(id, value); }

  };

}
#endif
