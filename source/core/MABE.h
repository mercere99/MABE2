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
#include "tools/vector_utils.h"

#include "OrganismType.h"
#include "Population.h"
#include "World.h"

namespace mabe {

  class MABE {
  private:
    /// Collect all world instances.  Each world will maintain its own environment
    /// (evaluate module), selection module, and populations of current organisms.
    emp::vector<mabe::World> worlds;

    /// Collect all organism types from all words.  Organism types have distinct
    /// names and can be manipulated as a whole.
    emp::vector<emp::Ptr<mabe::OrganismTypeBase>> org_types;

  public:
    MABE() { }
    MABE(const MABE &) = delete;
    MABE(MABE &&) = delete;
    ~MABE() {
      for (auto x : org_types) x.Delete();
    }

    int GetWorldID(const std::string & name) const {
      return emp::FindEval(worlds, [name](auto w){ return w.GetName() == name; });
    }
    int GetOrgTypeID(const std::string & name) const {
      return emp::FindEval(org_types, [name](auto o){ return o.GetName() == name; });
    }
  };

}

#endif
