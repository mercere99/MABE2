/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  SelectTournament.hpp
 *  @brief MABE module to enable tournament selection (choose T random orgs and return "best")
 */

#ifndef MABE_SELECT_TOURNAMENT_H
#define MABE_SELECT_TOURNAMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  /// Add elite selection with the current population.
  class SelectTournament : public Module {
  private:
    std::string fit_equation;     ///< Trait function that we should select on
    size_t tourny_size;      ///< Number of organisms in each tournament
    size_t num_tournies;     ///< Number of tournaments to run
    int select_pop_id = 0;   ///< Population that we are selecting from
    int birth_pop_id = 1;    ///< Population that births should go into

  public:
    SelectTournament(mabe::MABE & control,
                     const std::string & name="SelectTournament",
                     const std::string & desc="Module to select the top fitness organisms from random subgroups for replication.",
                     const std::string & in_fit="fitness",
                     size_t t_size=7, size_t num_t=1)
      : Module(control, name, desc)
      , fit_equation(in_fit), tourny_size(t_size), num_tournies(num_t)
    {
      SetSelectMod(true);              ///< Mark this module as a selection module.
    }
    ~SelectTournament() { }

    void SetupConfig() override {
      LinkPop(select_pop_id, "select_pop", "Population from which to select parents");
      LinkPop(birth_pop_id, "birth_pop", "Population into which offspring should be placed");
      LinkVar(tourny_size, "tournament_size", "Number of orgs in each tournament");
      LinkVar(num_tournies, "num_tournaments", "Number of tournaments to run");
      LinkVar(fit_equation, "fitness_fun", "Trait equation that produces fitness value to use");
    }

    void SetupModule() override {
      AddRequiredEquation(fit_equation); ///< The fitness traits must be set by another module.
    }

    void OnUpdate(size_t ud) override {
      control.Verbose("UD ", ud, ": Running SelectTournament::OnUpdate()");

      emp::Random & random = control.GetRandom();
      Population & select_pop = control.GetPopulation(select_pop_id);
      Population & birth_pop = control.GetPopulation(birth_pop_id);
      const size_t N = select_pop.GetSize();

      if (select_pop.GetNumOrgs() == 0) {
        AddError("Trying to run Tournament Selection on an Empty Population.");
        return;
      }

      // Setup the fitness function
      auto fit_fun = control.BuildTraitEquation(fit_equation);

      // @CAO if we have a sparse Population, we probably want to take that into account.

      // Loop through each round of tournament selection.
      for (size_t round = 0; round < num_tournies; round++) {
        // Find a random organism in the population and call it "best"
        size_t best_id = random.GetUInt(N);
        while (select_pop[best_id].IsEmpty()) best_id = random.GetUInt(N);
        double best_fit = fit_fun(select_pop[best_id].GetDataMap());

        // Loop through other organisms for the rest of the tournament size, and pick best.
        for (size_t test=1; test < tourny_size; test++) {
          size_t test_id = random.GetUInt(N);
          while (select_pop[test_id].IsEmpty()) test_id = random.GetUInt(N);
          double test_fit = fit_fun(select_pop[test_id].GetDataMap());          
          if (test_fit > best_fit) {
            best_id = test_id;
            best_fit = test_fit;
          }
        }

        // Replicate the organism that did best in this tournament.
        control.Replicate(select_pop.IteratorAt(best_id), birth_pop, 1);
      }

      control.Verbose(" - After ", num_tournies, " tournaments, select_pop has",
                      select_pop.GetNumOrgs(), "organisms and birth pop has",
                      birth_pop.GetNumOrgs(), ".");
    }

  };

  MABE_REGISTER_MODULE(SelectTournament, "Select the top fitness organisms from random subgroups for replication.");
}

#endif
