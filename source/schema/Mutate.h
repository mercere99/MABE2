/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Mutate.h
 *  @brief Default module to handle mutations.
 */

#ifndef MABE_SCHEMA_MUTATE_H
#define MABE_SCHEMA_MUTATE_H

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  /// Add elite selection with the current population.
  class Mutate : public Module {
  private:
    size_t pop_id = 0;  ///< Which population are we mutating?
    size_t skip = 0;    ///< How many organisms should we skip before mutating?

  public:
    Mutate(size_t _pop_id=0, size_t _skip=0)
      : Module("Mutate", "Module to trigger mutations in organisms"), pop_id(_pop_id), skip(_skip)
    {
      IsMutate(true);                   ///< Mark this module as a mutation module.
      DefaultSync();                    ///< This module defaults to synchronous generations.

      LinkConfigVar(skip, "skip", "Number of orgs to exempt from mutating");
    }

    void Update(mabe::World & world) {
      Population & pop = world.GetPopulation(pop_id);

      // Loop through the organisms (skipping any at the beginning that we need to) and
      // run Mutate() on each of them.
      for (auto it = pop.begin() + skip; it != pop.end(); it++) {
        if (it.IsOccupied()) it->Mutate(world.GetRandom());
      }
    }
  };

}

#endif