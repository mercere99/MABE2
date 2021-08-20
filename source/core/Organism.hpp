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
 *  template mabe::OrganismTeplate<ORG_T> is derived from mabe::OrgType and should be used as
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
#include "emp/bits/BitVector.hpp"
#include "emp/data/DataMap.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"

#include "OrgType.hpp"

namespace mabe {

  /// A generic base class implementing the use of dynamic traits via DataMaps.
  class AnnotatedType {
  private:
    emp::DataMap data_map;   ///< Dynamic variables assigned to this class.

  public:
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
  };

  class Organism : public OrgType, public AnnotatedType {
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




    // -- Also deal with some depricated functionality... --

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
