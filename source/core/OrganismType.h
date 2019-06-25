/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  OrganismType.h
 *  @brief Details about how a specific type of organism should function.
 */

#ifndef MABE_ORGANISM_TYPE_H
#define MABE_ORGANISM_TYPE_H

#include <functional>

#include "base/unordered_map.h"

namespace mabe {

  class Organism;

  class OrgTypeBase {
  private:
    std::string name;     ///< Name used for this type of organisms.
    emp::VarMap var_map;  ///< Map of run-time values associated with this organism type.

  public:
    OrgTypeBase(const std::string & in_name) : name(in_name) { ; }
    virtual ~OrgTypeBase() { ; }

    const std::string & GetName() const { return name; }

    virtual size_t MutateOrg(Organism & org) = 0;
  };

  template <typename ORG_T>
  class OrganismType : public OrgTypeBase {
  private:
    std::function<size_t(ORG_T &)> mut_fun;  // Function to mutate this type of organism.

  public:
    OrganismType(const std::string & in_name) : OrgTypeBase(in_name) {
      mut_fun = [](ORG_T &){ return 0; }; // No mutation function setup!
    }

    size_t MutateOrg(Organism & org) { return mut_fun(org); }
  };

}

#endif
