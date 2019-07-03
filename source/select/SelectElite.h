/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  SelectElite.h
 *  @brief MABE module to enable elite selection.
 */

#ifndef MABE_SELECT_ELITE_H
#define MABE_SELECT_ELITE_H

#include "../core/MABE.h"
#include "../core/Module.h"

#include "tools/reference_vector.h"

namespace mabe {

  /// Add elite selection with the current population.
  class SelectElite : public Module {
  private:
    std::string trait;   ///< Which trait should we select on?
    size_t top_count;    ///< Top how-many should we select?
    size_t copy_count;   ///< How many copies of each should we make?

  public:
    SelectElite(const std::string & in_trait="fitness") : trait(in_trait) {
      IsSelect(true);                  ///< Mark this module as a selection module.
      DefaultSync();                   ///< This module defaults to synchronous generations.
      AddRequiredTrait<double>(trait); ///< The fitness trait must be set by another module.
    }
    ~SelectElite() { }

    void Setup(mabe::World & world) {
    }

    void Update() {
    }
  };

}

#endif