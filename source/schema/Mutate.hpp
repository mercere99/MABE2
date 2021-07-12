/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  Mutate.hpp
 *  @brief Default module to handle mutations.
 */

#ifndef MABE_SCHEMA_MUTATE_H
#define MABE_SCHEMA_MUTATE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

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

    void OnUpdate(size_t /* update */) override {
      // @CAO: When we move this over to using collections, we can handle the skip more efficiently.

      Population & pop = control.GetPopulation(pop_id);

      // Loop through the organisms (skipping any at the beginning that we need to) and
      // run Mutate() on each of them.
      auto it = pop.begin();
      for (size_t i = 0; i < skip; i++) ++it;

      while (it != pop.end()) {
        if (it.IsOccupied()) it->Mutate(control.GetRandom());
        ++it;
      }
    }
  };

  MABE_REGISTER_MODULE(Mutate, "Trigger mutations in organisms.");
}

#endif
