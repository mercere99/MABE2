/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EmptyOrganism.hpp
 *  @brief Simple organism placeholder to represent open positions in a population.
 */

#ifndef MABE_EMPTY_ORGANISM_H
#define MABE_EMPTY_ORGANISM_H

#include "Organism.hpp"
#include "OrganismManager.hpp"

namespace mabe {

  /// An EmptyOrganism is used as a placeholder in an empty cell in a population.
  class EmptyOrganism : public Organism {
  public:
    EmptyOrganism(OrganismManager<EmptyOrganism> & _manager) : Organism(_manager) { ; }
    emp::Ptr<OrgType> Clone() const override { emp_error("Do not clone EmptyOrganism"); return nullptr; }
    std::string ToString() const override { return "[empty]"; }
    size_t Mutate(emp::Random &) override { emp_error("EmptyOrganism cannot Mutate()"); return -1; }
    void Randomize(emp::Random &) override { emp_error("EmptyOrganism cannot Randomize()"); }
    bool IsEmpty() const noexcept override { return true; }
  };

  class EmptyOrganismManager : public OrganismManager<EmptyOrganism> {
  public:
    EmptyOrganismManager(MABE & in_control, const std::string & in_name, const std::string & in_desc="")
      : OrganismManager<EmptyOrganism>(in_control, in_name, in_desc) { ; }
    ~EmptyOrganismManager() { ; }

    std::string GetTypeName() const override { return "EmptyOrganismManager"; }
    emp::TypeID GetObjType() const override { return emp::GetTypeID<EmptyOrganism>(); }

    emp::Ptr<OrgType> Make_impl() override { return emp::NewPtr<EmptyOrganism>(*this); }
    emp::Ptr<OrgType> Make_impl(emp::Random &) override { emp_error("Cannot make a 'random' EmptyOrganism."); return nullptr; }
  };

}

#endif
