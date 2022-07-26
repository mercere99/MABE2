/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  RandomReplacement.h
 *  @brief Each birth replaces a random organism in the population, keeping it at a constant size
 *
 *  When a birth occurs, the child is placed over an existing organism at random.
 *  This keeps the population size constant assuming there are no other deaths
 *  Organisms that are injected do NOT replace another org, so that can inflate population size
 * 
 *  When a neighbor position is requested, a random position from the entire population is
 *  returned.
 */

#ifndef MABE_RANDOM_REPLACEMENT_H
#define MABE_RANDOM_REPLACEMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  /// Organism births replace random orgs in the population, so pop stays at a constant size
  class RandomReplacement : public Module {
  private:
    Collection target_collect; ///< Collection of populations to manage

  public:
    RandomReplacement(mabe::MABE & control,
                    const std::string & name="RandomReplacement",
                    const std::string & desc="Module to place new organisms over random organisms.")
      : Module(control, name, desc), target_collect(control.GetPopulation(1))
    {
      SetPlacementMod(true);
    }
    ~RandomReplacement() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Population(s) to manage.");
    }

    /// Set birth and inject functions for the specified populations
    void SetupModule() override {
      for(size_t pop_id = 0; pop_id < control.GetNumPopulations(); ++pop_id){
        Population& pop = control.GetPopulation(pop_id);
        if(target_collect.HasPopulation(pop)){
          pop.SetPlaceBirthFun( 
            [this, &pop](Organism & /*org*/, OrgPosition ppos) {
              return PlaceBirth(ppos, pop);
            }
          );
          pop.SetPlaceInjectFun( 
            [this, &pop](Organism & /*org*/){
              return PlaceInject(pop);
            }
          );
        }
      }
    }

    /// Choose random position in population for organism to replace
    OrgPosition PlaceBirth(OrgPosition ppos, Population & target_pop) {
      // If the current position is monitored, return a random place in the population.
      if (target_collect.HasPopulation(target_pop)) {
        OrgPosition new_pos = 
            OrgPosition(target_pop, control.GetRandom().GetUInt(target_pop.GetSize()));
        while(new_pos == ppos){ // Do not allow parent to be replaced
          new_pos =OrgPosition(target_pop, control.GetRandom().GetUInt(target_pop.GetSize()));
        }
        return new_pos;
      }

      // Otherwise, don't find a legal place!
      return OrgPosition();      
    }

    /// Expand population for injected organism
    OrgPosition PlaceInject(Population & target_pop) {
      // If inject is going to a monitored population, place it in a new, empty cell!
      if (target_collect.HasPopulation(target_pop)) return control.PushEmpty(target_pop);

      // Otherwise, don't place!
      return OrgPosition();      
    }

  };

  MABE_REGISTER_MODULE(RandomReplacement, "Always appened births to the end of a population.");
}

#endif
