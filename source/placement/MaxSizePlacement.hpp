/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  MaxSizePlacement.h
 *  @brief Population grows up to a given size, then new births randomly replace existing orgs
 *
 *  When a birth occurs, we check if the population is at the maximum size. 
 *    If not, we add the append the new organism to the population. 
 *    If it is, the child is placed over an existing organism at random.
 * 
 *  When a neighbor position is requested, a random position from the entire population is
 *  returned.
 */

#ifndef MABE_MAX_SIZE_PLACEMENT_H
#define MABE_MAX_SIZE_PLACEMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  /// Grows population to a given size, then randomly places additional births over existing orgs
  class MaxSizePlacement : public Module {
  private:
    Collection target_collect; ///< Collection of populations to manage
    size_t max_pop_size;       ///< Maximum population size, at which additional births replace existing organisms

  public:
    MaxSizePlacement(mabe::MABE & control,
                    const std::string & name="MaxSizePlacement",
                    const std::string & desc="Grow population to a given size, then replace random orgs to maintain size")
      : Module(control, name, desc), target_collect(control.GetPopulation(0))
    {
      SetPlacementMod(true);
    }
    ~MaxSizePlacement() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Population(s) to manage.");
      LinkVar(max_pop_size, "max_pop_size", "Maximum size of the population.");
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

    /// Place a birth. Method depends on current population size
    OrgPosition PlaceBirth(OrgPosition ppos, Population & target_pop) {
      if (target_collect.HasPopulation(target_pop)) { // If population is monitored...
        // If population not full, add new position
        if(target_collect.GetSize() < max_pop_size) return control.PushEmpty(target_pop);
        else{ // If population full, return a random org's position
          OrgPosition new_pos = 
              OrgPosition(target_pop, control.GetRandom().GetUInt(target_pop.GetSize()));
          while(new_pos == ppos){ // Ensure we don't overwrite parent's position
            new_pos = OrgPosition(target_pop, control.GetRandom().GetUInt(target_pop.GetSize()));
          }
          return new_pos;
        }
      }

      // Otherwise, don't find a legal place!
      return OrgPosition();      
    }

    /// Manually inject an organism. Method depends on current population size
    OrgPosition PlaceInject(Population & target_pop) {
      if (target_collect.HasPopulation(target_pop)) { // If population is monitored...
        // If population not full, add new position
        if(target_collect.GetSize() < max_pop_size) return control.PushEmpty(target_pop);
        else{ // If population full, return a random org's position
          return OrgPosition(target_pop, control.GetRandom().GetUInt(target_pop.GetSize()));
        }
      }
      // Otherwise, don't find a legal place!
      return OrgPosition();      
    }

  };

  MABE_REGISTER_MODULE(MaxSizePlacement, "Grow population to a given size then maintain");
}

#endif
