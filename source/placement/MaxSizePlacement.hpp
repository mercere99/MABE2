/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  MaxSizePlacement.h
 *  @brief Population grows up to a given size, then new birth replace orgs and random.
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

  class MaxSizePlacement : public Module {
  private:
    Collection target_collect;
    size_t max_pop_size;

  public:
    MaxSizePlacement(mabe::MABE & control,
                    const std::string & name="MaxSizePlacement",
                    const std::string & desc="Module to place new organisms over random organisms.")
      : Module(control, name, desc), target_collect(control.GetPopulation(0))
    {
      SetPlacementMod(true);
    }
    ~MaxSizePlacement() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Population(s) to manage.");
      LinkVar(max_pop_size, "max_pop_size", "Maximum size of the population.");
    }

    void SetupModule() override {
      Population& pop = control.GetPopulation(0);
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

    OrgPosition PlaceBirth(OrgPosition ppos, Population & target_pop) {
      // If the current position is monitored, return a random place in the population.
      if (target_collect.HasPopulation(target_pop)) {
        if(target_collect.GetSize() < max_pop_size) return control.PushEmpty(target_pop);
        else{
          OrgPosition new_pos = 
              OrgPosition(target_pop, control.GetRandom().GetUInt(target_pop.GetSize()));
          while(new_pos == ppos)
            new_pos = OrgPosition(target_pop, control.GetRandom().GetUInt(target_pop.GetSize()));
          return new_pos;
        }
      }

      // Otherwise, don't find a legal place!
      return OrgPosition();      
    }

    OrgPosition PlaceInject(Population & target_pop) {
      // If the current position is monitored, return a random place in the population.
      if (target_collect.HasPopulation(target_pop)) {
        if(target_collect.GetSize() < max_pop_size) return control.PushEmpty(target_pop);
        else{
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
