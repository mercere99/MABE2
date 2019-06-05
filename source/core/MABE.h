/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  MABE.h
 *  @brief Master controller object for a MABE run.
 */

#ifndef MABE_MABE_H
#define MABE_MABE_H

#include <string>

#include "base/vector.h"

#include "OrganismType.h"
#include "Population.h"
#include "World.h"

namespace mabe {

  class MABE {
  private:
    /// Collect all world instances.  Each world will maintain its own environment
    /// (evaluate module), selection module, and populations of current organisms.
    emp::vector<mabe::World> world_map;

    /// Collect all organism types from all words.  Organism types have distinct
    /// names and can be manipulated as a whole.
    emp::vector<emp::Ptr<mabe::OrganismTypeBase>> orgtype_map;;
  };

}

#endif
