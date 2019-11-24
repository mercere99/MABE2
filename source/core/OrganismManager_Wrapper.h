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
#include "OrganismManager.h"

namespace mabe {

  template <typename ORG_T>
  class OrganismManager_Wrapper : public OrganismManager<ORG_T> {
  public:
    using org_t = ORG_T;
    using this_t = OrganismManager_Wrapper<ORG_T>;

    OrganismManager_Wrapper(const std::string & name)
    : OrganismManager<ORG_T>(name) { ; }

    ~OrganismManager_Wrapper() {
    }

  };
}

#endif
