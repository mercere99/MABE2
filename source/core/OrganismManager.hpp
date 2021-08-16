/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  OrganismManager.hpp
 *  @brief Track a category of organisms and maintain shared data within a category.
 */

#ifndef MABE_ORGANISM_MANAGER_H
#define MABE_ORGANISM_MANAGER_H

#include "ManagerModule.hpp"

namespace mabe {

  // Setup an OrganismManager as a standard ManagerModule that builds different kinds of Organisms
  template <typename ORG_T>
  using OrganismManager = ManagerModule<ORG_T, mabe::Organism>;

  // Setup OrganismTemplate as a quick way to build new organism types.
  template <typename ORG_T>
  using OrganismTemplate = ProductTemplate<ORG_T, mabe::Organism>;

  #define MABE_REGISTER_ORG_TYPE(TYPE, DESC) MABE_REGISTER_MANAGER_MODULE(TYPE, mabe::Organism, DESC)
}


#endif
