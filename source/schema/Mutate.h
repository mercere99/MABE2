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
    int pop_id = 0;   ///< Which population are we mutating?
    size_t skip = 0;  ///< How many organisms should we skip before mutating?

  public:
    Mutate(mabe::MABE & control,
           const std::string & name="Mutate",
           const std::string & desc="Module to trigger mutations in organisms",
           size_t _pop_id=0, size_t _skip=0)
      : Module(control, name, desc), pop_id(_pop_id), skip(_skip)
    {
      SetMutateMod(true);         ///< Mark this module as a mutation module.
    }

    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Which population should we mutate?");
      LinkVar(skip, "skip", "Number of orgs to exempt from mutating");
    }

    void OnUpdate(size_t update) override {
      Population & pop = control.GetPopulation(pop_id);

      // Loop through the organisms (skipping any at the beginning that we need to) and
      // run Mutate() on each of them.
      for (auto it = pop.begin() + skip; it != pop.end(); it++) {
        if (it.IsOccupied()) it->Mutate(control.GetRandom());
      }
    }
  };

  MABE_REGISTER_MODULE(Mutate, "Trigger mutations in organisms.");
}

#endif