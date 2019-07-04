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
#include "tools/valsort_map.h"

namespace mabe {

  /// Add elite selection with the current population.
  class SelectElite : public Module {
  private:
    std::string trait;   ///< Which trait should we select on?
    size_t top_count;    ///< Top how-many should we select?
    size_t copy_count;   ///< How many copies of each should we make?

  public:
    SelectElite(const std::string & in_trait="fitness", size_t tcount=1, size_t ccount=1)
      : trait(in_trait), top_count(tcount), copy_count(ccount)
    {
      IsSelect(true);                  ///< Mark this module as a selection module.
      DefaultSync();                   ///< This module defaults to synchronous generations.
      AddRequiredTrait<double>(trait); ///< The fitness trait must be set by another module.
      SetRequiredPops(1);              ///< Can only run elite selection on one pop at a time.
    }
    ~SelectElite() { }

    void Setup(mabe::World & world) {
      (void) world;
    }

    void Update() {
      // Construct a map of all IDs to their associated fitness values.
      emp::valsort_map<size_t, double> id_fit_map;
      for (size_t id = 0; id < pops[0].GetSize(); id++) {
        if (pops[0][id].IsEmpty()) continue;
        id_fit_map.Set(id, pops[0][id].GetVar<double>(trait));
      }

      // Loop through the IDs in fitness order (from highest), replicating each
      size_t num_reps = 0;
      for (auto it = id_fit_map.crvbegin(); it != id_fit_map.crvend() && num_reps < top_count; it++) {
        pops[0].Replicate(it->first, copy_count);
      }
    }
  };

}

#endif