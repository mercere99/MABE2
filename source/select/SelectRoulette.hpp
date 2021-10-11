/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  SelectRoulette.hpp
 *  @brief MABE module to enable roulette selection.
 */

#ifndef MABE_SELECT_ROULETTE_H
#define MABE_SELECT_ROULETTE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

#include "emp/datastructs/IndexMap.hpp"

namespace mabe {

  /// Add roulette selection with the current population.
  class SelectRoulette : public Module {
  private:
    std::string fitness_trait="fitness";  ///< Which trait should we select on?
    size_t select_count=1;                ///< How many times to run roulette?
    size_t copy_count=1;                  ///< How many copies of each should we make?
    int select_pop_id = 0;                ///< Which population are we selecting from?
    int birth_pop_id = 1;                 ///< Which population should births go into?

  public:
    SelectRoulette(
      mabe::MABE & control,
      const std::string & name="SelectRoulette",
      const std::string & desc="Module to choose random organisms for replication, based on fitness."
    ) : Module(control, name, desc)
    {
      SetSelectMod(true);               ///< Mark this module as a selection module.
    } 
    ~SelectRoulette() { }

    void SetupConfig() override {
      LinkPop(select_pop_id, "select_pop", "Which population should we select parents from?");
      LinkPop(birth_pop_id, "birth_pop", "Which population should births go into?");
      LinkVar(select_count, "select_count", "How many organisms should we choose to replicate?");
      LinkVar(copy_count, "copy_count", "Number of copies to make of replicated organisms");
      LinkVar(fitness_trait, "fitness_trait", "Which trait provides the fitness value to use?");
    }

    void SetupModule() override {
      AddRequiredTrait<double>(fitness_trait);  ///< The fitness trait must be set by another module.
    }

    void OnUpdate(size_t /* update */) override {
      if (select_pop_id == birth_pop_id) {
        AddError("For now, birth_pop and select_pop must be different.");
        return;
      }

      Population & select_pop = control.GetPopulation(select_pop_id);
      emp::IndexMap fit_map(select_pop.GetSize(), 0.0);
      for (size_t org_pos = 0; org_pos < select_pop.GetSize(); org_pos++) {
        if (select_pop.IsEmpty(org_pos)) continue;
        fit_map[org_pos] = select_pop[org_pos].GetTrait<double>(fitness_trait);
      }

      // Loop through picking IDs proportional to fitness_trait, replicating each
      Population & birth_pop = control.GetPopulation(birth_pop_id);
      emp::Random & random = control.GetRandom();
      for (size_t num_reps = 0; num_reps < select_count; num_reps++) {
        size_t org_id = fit_map.Index( random.GetDouble(fit_map.GetWeight()) );
        control.Replicate(select_pop.IteratorAt(org_id), birth_pop, copy_count);
      }
    }
  };

  MABE_REGISTER_MODULE(SelectRoulette, "Randomly choose organisms to replicate weighted by fitness.");
}

#endif
