/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
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
    double sharing_threshold; ///< How similar to organisms need to be for fitness sharing?
    double alpha = 1;        ///< Fitness sharing shape parameter

  public:
    SelectFitnessSharing(mabe::MABE & control,
                     const std::string & name="SelectFitnessSharing",
                     const std::string & desc="Module to select the top fitness organisms from random subgroups for replication.",
                     const std::string & in_trait="fitness",
                     const std::string & in_share_trait="vals",
                     size_t t_size=7)
      : Module(control, name, desc)
      , trait(in_trait), sharing_trait(in_share_trait), tourny_size(t_size)
    {
      SetSelectMod(true);              ///< Mark this module as a selection module.
    }
    ~SelectFitnessSharing() { }

    void SetupConfig() override {
      LinkVar(tourny_size, "tournament_size", "Number of orgs in each tournament");
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

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
        "SELECT",
        [](SelectFitnessSharing & mod, Population & from, Population & to, double count) {
          return mod.Select(from,to,count);
        },
        "Perform fitness sharing selection on the provided organisms.");
    }

    Collection Select(Population & select_pop, Population & birth_pop, size_t num_births) {
      emp::Random & random = control.GetRandom();
      const size_t N = select_pop.GetSize();

      // Track where all organisms are placed.
      Collection placement_list;

      if (select_pop.GetNumOrgs() == 0) {
        emp::notify::Error("Trying to run Tournament Selection on an Empty Population.");
        return placement_list;
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
      for (size_t round = 0; round < num_births; round++) {
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

        // Replicate the organism that did best in this tournament.
        placement_list += control.Replicate(select_pop.IteratorAt(best_id), birth_pop, 1);
      }

      return placement_list;
    }

  };

  MABE_REGISTER_MODULE(SelectFitnessSharing, "Select the top fitness organisms from random subgroups for replication.");
}

#endif
