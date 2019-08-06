/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  OrganismType_Wrapper.h
 *  @brief Wrapper to fill in many of the OrganismType details.
 */

#ifndef MABE_ORGANISM_TYPE_WRAPPER_H
#define MABE_ORGANISM_TYPE_WRAPPER_H

#include <functional>
#include <iostream>

#include "base/unordered_map.h"
#include "data/VarMap.h"
#include "tools/Random.h"

#include "Organism.h"

namespace mabe {

  template <typename ORG_T>
  class OrganismType_Wrapper : public OrganismType {
  public:
    using org_t = ORG_T;
    using this_t = OrganismType_Wrapper<ORG_T>;

    OrganismType_Wrapper(const std::string & name) : OrganismType(name) { ; }

    /// Convert this organism to the correct type (after ensuring that it is!)
    org_t & ConvertOrg(Organism & org) const {
      emp_assert(&(org.GetType()) == this);
      return (org_t &) org;
    }

    /// Convert this CONST organism to the correct type (after ensuring that it is!)
    const org_t & ConvertOrg(const Organism & org) const {
      emp_assert(&(org.GetType()) == this);
      return (org_t &) org;
    }

    /// Create a clone of the provided organism.
    emp::Ptr<Organism> CloneOrganism(const Organism & org) const override {
      return emp::NewPtr<org_t>( ConvertOrg(org) );
    }

    /// Crate a random organism from scratch.
    emp::Ptr<Organism> MakeOrganism(emp::Random & random) const override {
      auto org_ptr = emp::NewPtr<org_t>(this);
      Randomize(*org_ptr, random);
      return org_ptr;
    }

    /// By default print an organism by triggering it's ToString() function.
    std::ostream & Print(Organism & org, std::ostream & os) const override {
      emp_assert(&(org.GetType()) == this);
      os << org.ToString();
      return os;
    }
  };
}

#endif
