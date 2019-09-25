/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  SelectTournament.h
 *  @brief MABE module to enable tournament selection (choose T random orgs and return "best")
 */

#ifndef MABE_SELECT_TOURNAMENT_H
#define MABE_SELECT_TOURNAMENT_H

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  /// Add elite selection with the current population.
  class SelectTournament : public Module {
  private:
    std::string trait;   ///< Which trait should we select on?
    size_t tourny_size;  ///< How big should each tournament be?
    size_t num_tournies; ///< How many tournaments should we run?
    int pop_id = 0;      ///< Which population are we selecting from?

  public:
    SelectTournament(mabe::MABE & control,
                     const std::string & name="SelectTournament",
                     const std::string & desc="Module to select the top fitness organisms from random subgroups for replication.",
                     const std::string & in_trait="fitness",
                     size_t t_size=7, size_t num_t=1)
      : Module(control, name, desc)
      , trait(in_trait), tourny_size(t_size), num_tournies(num_t)
    {
      SetSelectMod(true);              ///< Mark this module as a selection module.
      AddRequiredTrait<double>(trait); ///< The fitness trait must be set by another module.
      SetMinPops(1);                   ///< Must run elite selection on a population.
    }
    ~SelectTournament() { }

    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Which population should we select parents from?");
      LinkVar(tourny_size, "tournament_size", "Number of orgs in each tournament", 7);
      LinkVar(num_tournies, "num_tournaments", "Number of tournaments to run", 1);
      LinkVar(trait, "fitness_trait", "Which trait provides the fitness value to use?", "fitness");
    }

    void SetupModule() override { }

    void OnUpdate(size_t update) override {
      emp::Random & random = control.GetRandom();
      Population & pop = control.GetPopulation(pop_id);
      size_t N = pop.GetSize();
      if (pop.GetNumOrgs() == 0) {
        control.AddError("Trying to run Tournament Selection on an Empty Population.");
        return;
      }

      // @CAO if we have a sparse population, we probably want to take that into account.

      // Loop through each round of tournament selection.
      for (size_t round = 0; round < num_tournies; round++) {
        size_t best_id = random.GetUInt(N);
        while (pop[best_id].IsEmpty()) best_id = random.GetUInt(N);
        double best_fit = pop[best_id].GetVar<double>(trait);

        for (size_t test=1; test < tourny_size; test++) {
          size_t test_id = random.GetUInt(N);
          while (pop[test_id].IsEmpty()) test_id = random.GetUInt(N);
          double test_fit = pop[test_id].GetVar<double>(trait);          
          if (test_fit > best_fit) {
            best_id = test_id;
            best_fit = test_fit;
          }
        }

        control.Replicate(pop.IteratorAt(best_id), 1);
      }
    }

  };

  MABE_REGISTER_MODULE(SelectTournament, "Select the top fitness organisms from random subgroups for replication.");
}

#endif