/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  SelectFitnessSharing.hpp
 *  @brief MABE module to enable tournament selection (choose T random orgs and return "best")
 */

#ifndef MABE_SELECT_FITNESS_SHARING_H
#define MABE_SELECT_FITNESS_SHARING_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

#include "emp/math/distances.hpp"

namespace mabe {

  /// Add elite selection with the current population.
  class SelectFitnessSharing : public Module {
  private:
    std::string trait;       ///< Which trait should we select on?
    std::string sharing_trait;  ///< Which trait should we use for sharing?
    size_t tourny_size;      ///< How big should each tournament be?
    size_t num_tournies;     ///< How many tournaments should we run?
    int select_pop_id = 0;   ///< Which population are we selecting from?
    int birth_pop_id = 1;    ///< Which population should births go into?
    double sharing_threshold; ///< How similar to organisms need to be for fitness sharing?
    double alpha = 1;        ///< Fitness sharing shape parameter

  public:
    SelectFitnessSharing(mabe::MABE & control,
                     const std::string & name="SelectFitnessSharing",
                     const std::string & desc="Module to select the top fitness organisms from random subgroups for replication.",
                     const std::string & in_trait="fitness",
                     const std::string & in_share_trait="vals",
                     size_t t_size=7, size_t num_t=1)
      : Module(control, name, desc)
      , trait(in_trait), sharing_trait(in_share_trait), tourny_size(t_size), num_tournies(num_t)
    {
      SetSelectMod(true);              ///< Mark this module as a selection module.
    }
    ~SelectFitnessSharing() { }

    void SetupConfig() override {
      LinkPop(select_pop_id, "select_pop", "Which population should we select parents from?");
      LinkPop(birth_pop_id, "birth_pop", "Which population should births go into?");
      LinkVar(tourny_size, "tournament_size", "Number of orgs in each tournament");
      LinkVar(num_tournies, "num_tournaments", "Number of tournaments to run");
      LinkVar(trait, "fitness_trait", "Which trait provides the fitness value to use?");
      LinkVar(sharing_trait, "sharing_trait", "Which trait should we do fitness sharing based on?");
      LinkVar(alpha, "alpha", "Sharing function exponent");
      LinkVar(sharing_threshold, "sharing_threshold", "How similar things need to be to share fitness");
    }

    void SetupModule() override {
      AddRequiredTrait<double>(trait); ///< The fitness trait must be set by another module.
      AddRequiredTrait<emp::vector<double>>(sharing_trait); ///< The fitness sharing trait must be set by another module.
      AddOwnedTrait<double>("shared_fitness", "Fitness sharing fitness", 0.0); // Place to store shared fitness
    }

    void OnUpdate(size_t ud) override {
      control.Verbose("UD ", ud, ": Running SelectFitnessSharing::OnUpdate()");

      emp::Random & random = control.GetRandom();
      Population & select_pop = control.GetPopulation(select_pop_id);
      Population & birth_pop = control.GetPopulation(birth_pop_id);
      const size_t N = select_pop.GetSize();

      if (select_pop.GetNumOrgs() == 0) {
        AddError("Trying to run Tournament Selection on an Empty Population.");
        return;
      }

      for (int i = 0; i < select_pop.size(); i++) {
        if (select_pop.IsEmpty(i)) {
          continue;
        }
        Organism & org1 = select_pop[i];
        double niche_count = 0.1;
        select_pop[i].GenerateOutput();
        for (int j = 0; j < select_pop.size(); j++) {
          if (select_pop.IsEmpty(j) || i == j) {
            continue;
          }
          Organism & org2 = select_pop[j];          
          double dist = emp::EuclideanDistance(org1.GetTrait<emp::vector<double> >(sharing_trait), org2.GetTrait<emp::vector<double> >(sharing_trait));
          niche_count += std::max(1.0 - std::pow(dist/sharing_threshold, alpha), 0.0);
        }
        // std::cout <<  emp::to_string(org1.GetTrait<emp::vector<double>>("scores")) << " " << niche_count << " " << org1.GetTrait<double>(trait) << " " <<  org1.GetTrait<double>(trait)/niche_count << " " << std::endl;        
        org1.SetTrait("shared_fitness", org1.GetTrait<double>(trait)/niche_count);
      }    

      // @CAO if we have a sparse Population, we probably want to take that into account.

      // Loop through each round of tournament selection.
      for (size_t round = 0; round < num_tournies; round++) {
        // Find a random organism in the population and call it "best"
        size_t best_id = random.GetUInt(N);
        while (select_pop[best_id].IsEmpty()) best_id = random.GetUInt(N);
        double best_fit = select_pop[best_id].GetTrait<double>(sharing_trait);

        // Loop through other organisms for the rest of the tournament size, and pick best.
        for (size_t test=1; test < tourny_size; test++) {
          size_t test_id = random.GetUInt(N);
          while (select_pop[test_id].IsEmpty()) test_id = random.GetUInt(N);
          double test_fit = select_pop[test_id].GetTrait<double>(sharing_trait);          
          if (test_fit > best_fit) {
            best_id = test_id;
            best_fit = test_fit;
          }
        }

        // Replicat the organism that did best in this tournament.
        control.Replicate(select_pop.IteratorAt(best_id), birth_pop, 1);
      }

      control.Verbose(" - After ", num_tournies, " tournaments, select_pop has",
                      select_pop.GetNumOrgs(), "organisms and birth pop has",
                      birth_pop.GetNumOrgs(), ".");
    }

  };

  MABE_REGISTER_MODULE(SelectFitnessSharing, "Select the top fitness organisms from random subgroups for replication.");
}

#endif
