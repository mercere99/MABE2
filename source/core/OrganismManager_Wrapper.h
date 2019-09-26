/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  OrganismManager_Wrapper.h
 *  @brief Wrapper to fill in many of the OrganismManager details.
 */

#ifndef MABE_ORGANISM_MANAGER_WRAPPER_H
#define MABE_ORGANISM_MANAGER_WRAPPER_H

#include <functional>
#include <iostream>

#include "base/unordered_map.h"
#include "data/VarMap.h"
#include "tools/Random.h"

#include "Organism.h"

namespace mabe {

  template <typename ORG_T>
  class OrganismManager_Wrapper : public OrganismManager {
  public:
    using org_t = ORG_T;
    using this_t = OrganismManager_Wrapper<ORG_T>;

    OrganismManager_Wrapper(const std::string & name)
    : OrganismManager(name) {
      prototype = emp::NewPtr<org_t>(this);
    }

    ~OrganismManager_Wrapper() {
      prototype.Delete();
    }

    /// Convert this organism to the correct type (after ensuring that it is!)
    org_t & ConvertOrg(Organism & org) const {
      emp_assert(&(org.GetManager()) == this);
      return (org_t &) org;
    }

    /// Convert this CONST organism to the correct type (after ensuring that it is!)
    const org_t & ConvertOrg(const Organism & org) const {
      emp_assert(&(org.GetManager()) == this);
      return (org_t &) org;
    }

    /// Create a clone of the provided organism; default to using copy constructor.
    emp::Ptr<Organism> CloneOrganism(const Organism & org) const override {
      return emp::NewPtr<org_t>( ConvertOrg(org) );
    }

    /// Create a random organism from scratch.  Default to using the prototype organism.
    emp::Ptr<Organism> MakeOrganism() const override {
      auto org_ptr = prototype->Clone();
      return org_ptr;
    }

    /// Create a random organism from scratch.  Default to using the prototype organism
    /// and then randomize if a random number generator is provided.
    emp::Ptr<Organism> MakeOrganism(emp::Random & random) const override {
      auto org_ptr = prototype->Clone();
      org_ptr->Randomize(random);
      return org_ptr;
    }

    /// Convert an organism to a string for printing; if not overridden, just prints
    /// "__unknown__".
    std::string ToString(const Organism &) const override { return "__unknown__"; };

    /// By default print an organism by triggering it's ToString() function.
    std::ostream & Print(Organism & org, std::ostream & os) const override {
      emp_assert(&(org.GetManager()) == this);
      os << org.ToString();
      return os;
    }

    void SetupConfig() override {
      prototype->SetupConfig();
    }

  };
}

#endif
